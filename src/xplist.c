/*
 * xplist.c
 * XML plist implementation
 *
 * Copyright (c) 2010-2016 Nikias Bassen All Rights Reserved.
 * Copyright (c) 2010-2015 Martin Szulecki All Rights Reserved.
 * Copyright (c) 2008 Jonathan Beck All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRPTIME
#define _XOPEN_SOURCE 600
#endif

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <inttypes.h>
#include <math.h>

#include <node.h>
#include <node_list.h>
#include <node_iterator.h>

#include "plist.h"
#include "base64.h"
#include "strbuf.h"
#include "time64.h"

#define XPLIST_KEY	"key"
#define XPLIST_KEY_LEN 3
#define XPLIST_FALSE	"false"
#define XPLIST_FALSE_LEN 5
#define XPLIST_TRUE	"true"
#define XPLIST_TRUE_LEN 4
#define XPLIST_INT	"integer"
#define XPLIST_INT_LEN 7
#define XPLIST_REAL	"real"
#define XPLIST_REAL_LEN 4
#define XPLIST_DATE	"date"
#define XPLIST_DATE_LEN 4
#define XPLIST_DATA	"data"
#define XPLIST_DATA_LEN 4
#define XPLIST_STRING	"string"
#define XPLIST_STRING_LEN 6
#define XPLIST_ARRAY	"array"
#define XPLIST_ARRAY_LEN 5
#define XPLIST_DICT	"dict"
#define XPLIST_DICT_LEN 4

#define MAC_EPOCH 978307200

static const char XML_PLIST_PROLOG[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version=\"1.0\">\n";
static const char XML_PLIST_EPILOG[] = "</plist>\n";

#ifdef DEBUG
static int plist_xml_debug = 0;
#define PLIST_XML_ERR(...) if (plist_xml_debug) { fprintf(stderr, "libplist[xmlparser] ERROR: " __VA_ARGS__); }
#else
#define PLIST_XML_ERR(...)
#endif

void plist_xml_init(void)
{
    /* init XML stuff */
#ifdef DEBUG
    char *env_debug = getenv("PLIST_XML_DEBUG");
    if (env_debug && !strcmp(env_debug, "1")) {
        plist_xml_debug = 1;
    }
#endif
}

void plist_xml_deinit(void)
{
    /* deinit XML stuff */
}

static size_t dtostr(char *buf, size_t bufsize, double realval)
{
    double f = realval;
    double ip = 0.0;
    int64_t v;
    size_t len;
    size_t p;

    f = modf(f, &ip);
    len = snprintf(buf, bufsize, "%s%"PRIi64, ((f < 0) && (ip >= 0)) ? "-" : "", (int64_t)ip);
    if (len >= bufsize) {
        return 0;
    }

    if (f < 0) {
        f *= -1;
    }
    f += 0.0000004;

    p = len;
    buf[p++] = '.';

    while (p < bufsize && (p <= len+6)) {
        f = modf(f*10, &ip);
        v = (int)ip;
        buf[p++] = (v + 0x30);
    }
    buf[p] = '\0';
    return p;
}

static void node_to_xml(node_t* node, bytearray_t **outbuf, uint32_t depth)
{
    plist_data_t node_data = NULL;

    char isStruct = FALSE;
    char tagOpen = FALSE;

    const char *tag = NULL;
    size_t tag_len = 0;
    char *val = NULL;
    size_t val_len = 0;

    uint32_t i = 0;

    if (!node)
        return;

    node_data = plist_get_data(node);

    switch (node_data->type)
    {
    case PLIST_BOOLEAN:
    {
        if (node_data->boolval) {
            tag = XPLIST_TRUE;
            tag_len = XPLIST_TRUE_LEN;
        } else {
            tag = XPLIST_FALSE;
            tag_len = XPLIST_FALSE_LEN;
        }
    }
    break;

    case PLIST_UINT:
        tag = XPLIST_INT;
        tag_len = XPLIST_INT_LEN;
        val = (char*)malloc(64);
        if (node_data->length == 16) {
            val_len = snprintf(val, 64, "%"PRIu64, node_data->intval);
        } else {
            val_len = snprintf(val, 64, "%"PRIi64, node_data->intval);
        }
        break;

    case PLIST_REAL:
        tag = XPLIST_REAL;
        tag_len = XPLIST_REAL_LEN;
        val = (char*)malloc(64);
        val_len = dtostr(val, 64, node_data->realval);
        break;

    case PLIST_STRING:
        tag = XPLIST_STRING;
        tag_len = XPLIST_STRING_LEN;
        /* contents processed directly below */
        break;

    case PLIST_KEY:
        tag = XPLIST_KEY;
        tag_len = XPLIST_KEY_LEN;
        /* contents processed directly below */
        break;

    case PLIST_DATA:
        tag = XPLIST_DATA;
        tag_len = XPLIST_DATA_LEN;
        /* contents processed directly below */
        break;
    case PLIST_ARRAY:
        tag = XPLIST_ARRAY;
        tag_len = XPLIST_ARRAY_LEN;
        isStruct = TRUE;
        break;
    case PLIST_DICT:
        tag = XPLIST_DICT;
        tag_len = XPLIST_DICT_LEN;
        isStruct = TRUE;
        break;
    case PLIST_DATE:
        tag = XPLIST_DATE;
        tag_len = XPLIST_DATE_LEN;
        {
            Time64_T timev = (Time64_T)node_data->realval + MAC_EPOCH;
            struct TM _btime;
            struct TM *btime = gmtime64_r(&timev, &_btime);
            if (btime) {
                val = (char*)malloc(24);
                memset(val, 0, 24);
                struct tm _tmcopy;
                copy_TM64_to_tm(btime, &_tmcopy);
                val_len = strftime(val, 24, "%Y-%m-%dT%H:%M:%SZ", &_tmcopy);
                if (val_len <= 0) {
                    free (val);
                    val = NULL;
                }
            }
        }
        break;
    case PLIST_UID:
        tag = XPLIST_DICT;
        tag_len = XPLIST_DICT_LEN;
        val = (char*)malloc(64);
        if (node_data->length == 16) {
            val_len = snprintf(val, 64, "%"PRIu64, node_data->intval);
        } else {
            val_len = snprintf(val, 64, "%"PRIi64, node_data->intval);
        }
        break;
    default:
        break;
    }

    for (i = 0; i < depth; i++) {
        str_buf_append(*outbuf, "\t", 1);
    }

    /* append tag */
    str_buf_append(*outbuf, "<", 1);
    str_buf_append(*outbuf, tag, tag_len);
    if (node_data->type == PLIST_STRING || node_data->type == PLIST_KEY) {
        size_t j;
        size_t len;
        off_t start = 0;
        off_t cur = 0;

        str_buf_append(*outbuf, ">", 1);
        tagOpen = TRUE;

        /* make sure we convert the following predefined xml entities */
        /* < = &lt; > = &gt; & = &amp; */
        len = node_data->length;
        for (j = 0; j < len; j++) {
            switch (node_data->strval[j]) {
            case '<':
                str_buf_append(*outbuf, node_data->strval + start, cur - start);
                str_buf_append(*outbuf, "&lt;", 4);
                start = cur+1;
                break;
            case '>':
                str_buf_append(*outbuf, node_data->strval + start, cur - start);
                str_buf_append(*outbuf, "&gt;", 4);
                start = cur+1;
                break;
            case '&':
                str_buf_append(*outbuf, node_data->strval + start, cur - start);
                str_buf_append(*outbuf, "&amp;", 5);
                start = cur+1;
                break;
            default:
                break;
            }
            cur++;
        }
        str_buf_append(*outbuf, node_data->strval + start, cur - start);
    } else if (node_data->type == PLIST_DATA) {
        str_buf_append(*outbuf, ">", 1);
        tagOpen = TRUE;
        str_buf_append(*outbuf, "\n", 1);
        if (node_data->length > 0) {
            char *buf = malloc(80);
            uint32_t j = 0;
            uint32_t indent = (depth > 8) ? 8 : depth;
            uint32_t maxread = ((76 - indent*8) / 4) * 3;
            size_t count = 0;
            size_t b64count = 0;
            while (j < node_data->length) {
                for (i = 0; i < indent; i++) {
                    str_buf_append(*outbuf, "\t", 1);
                }
                count = (node_data->length-j < maxread) ? node_data->length-j : maxread;
                b64count = base64encode(buf, node_data->buff + j, count);
                str_buf_append(*outbuf, buf, b64count);
                str_buf_append(*outbuf, "\n", 1);
                j+=count;
            }
            free(buf);
        }
        for (i = 0; i < depth; i++) {
            str_buf_append(*outbuf, "\t", 1);
        }
    } else if (node_data->type == PLIST_UID) {
        /* special case for UID nodes: create a DICT */
        str_buf_append(*outbuf, ">", 1);
        tagOpen = TRUE;
        str_buf_append(*outbuf, "\n", 1);

        /* add CF$UID key */
        for (i = 0; i < depth+1; i++) {
            str_buf_append(*outbuf, "\t", 1);
        }
        str_buf_append(*outbuf, "<key>CF$UID</key>", 17);
        str_buf_append(*outbuf, "\n", 1);

        /* add UID value */
        for (i = 0; i < depth+1; i++) {
            str_buf_append(*outbuf, "\t", 1);
        }
        str_buf_append(*outbuf, "<integer>", 9);
        str_buf_append(*outbuf, val, val_len);
        str_buf_append(*outbuf, "</integer>", 10);
        str_buf_append(*outbuf, "\n", 1);

        for (i = 0; i < depth; i++) {
            str_buf_append(*outbuf, "\t", 1);
        }
    } else if (val) {
        str_buf_append(*outbuf, ">", 1);
        tagOpen = TRUE;
        str_buf_append(*outbuf, val, val_len);
    } else if (isStruct) {
        tagOpen = TRUE;
        str_buf_append(*outbuf, ">", 1);
    } else {
        tagOpen = FALSE;
        str_buf_append(*outbuf, "/>", 2);
    }
    free(val);

    /* add return for structured types */
    if (node_data->type == PLIST_ARRAY || node_data->type == PLIST_DICT)
        str_buf_append(*outbuf, "\n", 1);

    if (isStruct) {
        node_iterator_t *ni = node_iterator_create(node->children);
        node_t *ch;
        while ((ch = node_iterator_next(ni))) {
            node_to_xml(ch, outbuf, depth+1);
        }
        node_iterator_destroy(ni);
    }

    /* fix indent for structured types */
    if (node_data->type == PLIST_ARRAY || node_data->type == PLIST_DICT) {
        for (i = 0; i < depth; i++) {
            str_buf_append(*outbuf, "\t", 1);
        }
    }

    if (tagOpen) {
        /* add closing tag */
        str_buf_append(*outbuf, "</", 2);
        str_buf_append(*outbuf, tag, tag_len);
        str_buf_append(*outbuf, ">", 1);
    }
    str_buf_append(*outbuf, "\n", 1);

    return;
}

static void parse_date(const char *strval, struct TM *btime)
{
    if (!btime) return;
    memset(btime, 0, sizeof(struct tm));
    if (!strval) return;
#ifdef HAVE_STRPTIME
    strptime((char*)strval, "%Y-%m-%dT%H:%M:%SZ", btime);
#else
#ifdef USE_TM64
    #define PLIST_SSCANF_FORMAT "%lld-%d-%dT%d:%d:%dZ"
#else
    #define PLIST_SSCANF_FORMAT "%d-%d-%dT%d:%d:%dZ"
#endif
    sscanf(strval, PLIST_SSCANF_FORMAT, &btime->tm_year, &btime->tm_mon, &btime->tm_mday, &btime->tm_hour, &btime->tm_min, &btime->tm_sec);
    btime->tm_year-=1900;
    btime->tm_mon--;
#endif
    btime->tm_isdst=0;
}

PLIST_API void plist_to_xml(plist_t plist, char **plist_xml, uint32_t * length)
{
    strbuf_t *outbuf = str_buf_new();

    str_buf_append(outbuf, XML_PLIST_PROLOG, sizeof(XML_PLIST_PROLOG)-1);

    node_to_xml(plist, &outbuf, 0);

    str_buf_append(outbuf, XML_PLIST_EPILOG, sizeof(XML_PLIST_EPILOG));

    *plist_xml = outbuf->data;
    *length = outbuf->len - 1;

    outbuf->data = NULL;
    str_buf_free(outbuf);
}

struct _parse_ctx {
    const char *pos;
    const char *end;
    int err;
};
typedef struct _parse_ctx* parse_ctx;

static void parse_skip_ws(parse_ctx ctx)
{
    while (ctx->pos < ctx->end && ((*(ctx->pos) == ' ') || (*(ctx->pos) == '\t') || (*(ctx->pos) == '\r') || (*(ctx->pos) == '\n'))) {
        ctx->pos++;
    }
}

static void find_char(parse_ctx ctx, char c, int skip_quotes)
{
    while (ctx->pos < ctx->end && (*(ctx->pos) != c)) {
        if (skip_quotes && (c != '"') && (*(ctx->pos) == '"')) {
            ctx->pos++;
            find_char(ctx, '"', 0);
            if (*(ctx->pos) != '"') {
                PLIST_XML_ERR("Unmatched double quote\n");
                return;
            }
        }
        ctx->pos++;
    }
}

static void find_str(parse_ctx ctx, const char *str, size_t len, int skip_quotes)
{
    while (ctx->pos < (ctx->end - len)) {
        if (!strncmp(ctx->pos, str, len)) {
            break;
        }
        if (skip_quotes && (*(ctx->pos) == '"')) {
            ctx->pos++;
            find_char(ctx, '"', 0);
            if (*(ctx->pos) != '"') {
                PLIST_XML_ERR("Unmatched double quote\n");
                return;
            }
        }
        ctx->pos++;
    }
}

static void find_next(parse_ctx ctx, const char *nextchars, int numchars, int skip_quotes)
{
    int i = 0;
    while (ctx->pos < ctx->end) {
        if (skip_quotes && (*(ctx->pos) == '"')) {
            ctx->pos++;
            find_char(ctx, '"', 0);
            if (*(ctx->pos) != '"') {
                PLIST_XML_ERR("Unmatched double quote\n");
                return;
            }
        }
        for (i = 0; i < numchars; i++) {
            if (*(ctx->pos) == nextchars[i]) {
                return;
            }
        }
        ctx->pos++;
    }
}

typedef struct {
    const char *begin;
    size_t length;
    int is_cdata;
    void *next;
} text_part_t;

static text_part_t* text_part_init(text_part_t* part, const char *begin, size_t length, int is_cdata)
{
    part->begin = begin;
    part->length = length;
    part->is_cdata = is_cdata;
    part->next = NULL;
    return part;
}

static void text_parts_free(text_part_t *tp)
{
    while (tp) {
        text_part_t *tmp = tp;
        tp = tp->next;
        free(tmp);
    }
}

static text_part_t* text_part_append(text_part_t* parts, const char *begin, size_t length, int is_cdata)
{
    text_part_t* newpart = malloc(sizeof(text_part_t));
    assert(newpart);
    parts->next = text_part_init(newpart, begin, length, is_cdata);
    return newpart;
}

static text_part_t* get_text_parts(parse_ctx ctx, const char* tag, size_t tag_len, int skip_ws, text_part_t *parts)
{
    const char *p = NULL;
    const char *q = NULL;
    text_part_t *last = NULL;

    if (skip_ws) {
        parse_skip_ws(ctx);
    }
    do {
        p = ctx->pos;
        find_char(ctx, '<', 0);
        if (*ctx->pos != '<') {
            PLIST_XML_ERR("EOF while looking for closing tag\n");
            ctx->err++;
            return NULL;
        }
        q = ctx->pos;
        ctx->pos++;
        if (ctx->pos >= ctx->end) {
            PLIST_XML_ERR("EOF while parsing '%s'\n", p);
            ctx->err++;
            return NULL;
        }
        if (*ctx->pos == '!') {
            ctx->pos++;
            if (*ctx->pos == '-' && *(ctx->pos+1) == '-') {
                if (last) {
                    last = text_part_append(last, p, q-p, 0);
                } else if (parts) {
                    last = text_part_init(parts, p, q-p, 0);
                }
                ctx->pos += 2;
                find_str(ctx, "-->", 3, 0);
                if (ctx->pos >= ctx->end || strncmp(ctx->pos, "-->", 3) != 0) {
                    PLIST_XML_ERR("EOF while looking for end of comment\n");
                    ctx->err++;
                    return NULL;
                }
                ctx->pos += 3;
            } else if (*ctx->pos == '[') {
                ctx->pos++;
                if (ctx->pos >= ctx->end - 8) {
                    PLIST_XML_ERR("EOF while parsing <[ tag\n");
                    ctx->err++;
                    return NULL;
                }
                if (strncmp(ctx->pos, "CDATA[", 6) == 0) {
                    if (q-p > 0) {
                        if (last) {
                            last = text_part_append(last, p, q-p, 0);
                        } else if (parts) {
                            last = text_part_init(parts, p, q-p, 0);
                        }
                    }
                    ctx->pos+=6;
                    p = ctx->pos;
                    find_str(ctx, "]]>", 3, 0);
                    if (ctx->pos >= ctx->end || strncmp(ctx->pos, "]]>", 3) != 0) {
                        PLIST_XML_ERR("EOF while looking for end of CDATA block\n");
                        ctx->err++;
                        return NULL;
                    }
                    q = ctx->pos;
                    if (last) {
                        last = text_part_append(last, p, q-p, 1);
                    } else if (parts) {
                        last = text_part_init(parts, p, q-p, 1);
                    }
                    ctx->pos += 3;
                } else {
                    p = ctx->pos;
                    find_next(ctx, " \r\n\t>", 5, 1);
                    PLIST_XML_ERR("Invalid special tag <[%.*s> encountered inside <%s> tag\n", (int)(ctx->pos - p), p, tag);
                    ctx->err++;
                    return NULL;
                }
            } else {
                p = ctx->pos;
                find_next(ctx, " \r\n\t>", 5, 1);
                PLIST_XML_ERR("Invalid special tag <!%.*s> encountered inside <%s> tag\n", (int)(ctx->pos - p), p, tag);
                ctx->err++;
                return NULL;
            }
        } else if (*ctx->pos == '/') {
            break;
        } else {
            p = ctx->pos;
            find_next(ctx, " \r\n\t>", 5, 1);
            PLIST_XML_ERR("Invalid tag <%.*s> encountered inside <%s> tag\n", (int)(ctx->pos - p), p, tag);
            ctx->err++;
            return NULL;
        }
    } while (1);
    ctx->pos++;
    if (ctx->pos >= ctx->end-tag_len || strncmp(ctx->pos, tag, tag_len)) {
        PLIST_XML_ERR("EOF or end tag mismatch\n");
        ctx->err++;
        return NULL;
    }
    ctx->pos+=tag_len;
    parse_skip_ws(ctx);
    if (ctx->pos >= ctx->end) {
        PLIST_XML_ERR("EOF while parsing closing tag\n");
        ctx->err++;
        return NULL;
    } else if (*ctx->pos != '>') {
        PLIST_XML_ERR("Invalid closing tag; expected '>', found '%c'\n", *ctx->pos);
        ctx->err++;
        return NULL;
    }
    ctx->pos++;

    if (q-p > 0) {
        if (last) {
            last = text_part_append(last, p, q-p, 0);
        } else if (parts) {
            last = text_part_init(parts, p, q-p, 0);
        }
    }
    return parts;
}

static int unescape_entities(char *str, size_t *length)
{
    size_t i = 0;
    size_t len = *length;
    while (len > 0 && i < len-1) {
        if (str[i] == '&') {
            char *entp = str + i + 1;
            while (i < len && str[i] != ';') {
                i++;
            }
            if (i >= len) {
                PLIST_XML_ERR("Invalid entity sequence encountered (missing terminating ';')\n");
                return -1;
            }
            if (str+i >= entp+1) {
                int entlen = str+i - entp;
                int bytelen = 1;
                if (!strncmp(entp, "amp", 3)) {
                    /* the '&' is already there */
                } else if (!strncmp(entp, "apos", 4)) {
                    *(entp-1) = '\'';
                } else if (!strncmp(entp, "quot", 4)) {
                    *(entp-1) = '"';
                } else if (!strncmp(entp, "lt", 2)) {
                    *(entp-1) = '<';
                } else if (!strncmp(entp, "gt", 2)) {
                    *(entp-1) = '>';
                } else if (*entp == '#') {
                    /* numerical  character reference */
                    uint64_t val = 0;
                    char* ep = NULL;
                    if (entlen > 8) {
                        PLIST_XML_ERR("Invalid numerical character reference encountered, sequence too long: &%.*s;\n", entlen, entp);
                        return -1;
                    }
                    if (*(entp+1) == 'x' || *(entp+1) == 'X') {
                        if (entlen < 3) {
                            PLIST_XML_ERR("Invalid numerical character reference encountered, sequence too short: &%.*s;\n", entlen, entp);
                            return -1;
                        }
                        val = strtoull(entp+2, &ep, 16);
                    } else {
                        if (entlen < 2) {
                            PLIST_XML_ERR("Invalid numerical character reference encountered, sequence too short: &%.*s;\n", entlen, entp);
                            return -1;
                        }
                        val = strtoull(entp+1, &ep, 10);
                    }
                    if (val == 0 || val > 0x10FFFF || ep-entp != entlen) {
                        PLIST_XML_ERR("Invalid numerical character reference found: &%.*s;\n", entlen, entp);
                        return -1;
                    }
                    /* convert to UTF8 */
                    if (val >= 0x10000) {
                        /* four bytes */
                        *(entp-1) = (char)(0xF0 + ((val >> 18) & 0x7));
                        *(entp+0) = (char)(0x80 + ((val >> 12) & 0x3F));
                        *(entp+1) = (char)(0x80 + ((val >> 6) & 0x3F));
                        *(entp+2) = (char)(0x80 + (val & 0x3F));
                        entp+=3;
                        bytelen = 4;
                    } else if (val >= 0x800) {
                        /* three bytes */
                        *(entp-1) = (char)(0xE0 + ((val >> 12) & 0xF));
                        *(entp+0) = (char)(0x80 + ((val >> 6) & 0x3F));
                        *(entp+1) = (char)(0x80 + (val & 0x3F));
                        entp+=2;
                        bytelen = 3;
                    } else if (val >= 0x80) {
                        /* two bytes */
                        *(entp-1) = (char)(0xC0 + ((val >> 6) & 0x1F));
                        *(entp+0) = (char)(0x80 + (val & 0x3F));
                        entp++;
                        bytelen = 2;
                    } else {
                        /* one byte */
                        *(entp-1) = (char)(val & 0x7F);
                    }
                } else {
                    PLIST_XML_ERR("Invalid entity encountered: &%.*s;\n", entlen, entp);
                    return -1;
                }
                memmove(entp, str+i+1, len - i);
                i -= entlen+1 - bytelen;
                len -= entlen+2 - bytelen;
                continue;
            } else {
                PLIST_XML_ERR("Invalid empty entity sequence &;\n");
                return -1;
            }
        }
        i++;
    }
    *length = len;
    return 0;
}

static char* text_parts_get_content(text_part_t *tp, int unesc_entities, size_t *length, int *requires_free)
{
    char *str = NULL;
    size_t total_length = 0;

    if (!tp) {
        return NULL;
    }
    char *p;
    if (requires_free && !tp->next) {
        if (tp->is_cdata || !unesc_entities) {
            *requires_free = 0;
            if (length) {
                *length = tp->length;
            }
            return (char*)tp->begin;
        }
    }
    text_part_t *tmp = tp;
    while (tp && tp->begin) {
        total_length += tp->length;
        tp = tp->next;
    }
    str = malloc(total_length + 1);
    assert(str);
    p = str;
    tp = tmp;
    while (tp && tp->begin) {
        size_t len = tp->length;
        strncpy(p, tp->begin, len);
        if (!tp->is_cdata && unesc_entities) {
            if (unescape_entities(p, &len) < 0) {
                free(str);
                return NULL;
            }
        }
        p += len;
        tp = tp->next;
    }
    *p = '\0';
    if (length) {
        *length = p - str;
    }
    if (requires_free) {
        *requires_free = 1;
    }
    return str;
}

static void node_from_xml(parse_ctx ctx, plist_t *plist, uint32_t depth)
{
    char *keyname = NULL;
    const char *p = NULL;
    while (ctx->pos < ctx->end && !ctx->err) {
        parse_skip_ws(ctx);
        if (ctx->pos >= ctx->end) {
            break;
        }
        if (*ctx->pos != '<') {
            p = ctx->pos;
            find_next(ctx, " \t\r\n", 4, 0);
            PLIST_XML_ERR("Expected: opening tag, found: %.*s\n", (int)(ctx->pos - p), p);
            ctx->pos = ctx->end;
            ctx->err++;
            break;
        }
        ctx->pos++;
        if (ctx->pos >= ctx->end) {
            break;
        }

        if (*(ctx->pos) == '?') {
            find_str(ctx, "?>", 2, 1);
            if (ctx->pos >= ctx->end) {
                break;
            }
            if (strncmp(ctx->pos, "?>", 2)) {
                PLIST_XML_ERR("Couldn't find <? tag closing marker\n");
                ctx->pos = ctx->end;
                return;
            }
            ctx->pos += 2;
            continue;
        } else if (*(ctx->pos) == '!') {
            /* comment or DTD */
            if (((ctx->end - ctx->pos) > 3) && !strncmp(ctx->pos, "!--", 3)) {
                ctx->pos += 3;
                find_str(ctx,"-->", 3, 0);
                if (strncmp(ctx->pos, "-->", 3)) {
                    PLIST_XML_ERR("Couldn't find end of comment\n");
                    ctx->pos = ctx->end;
                    return;
                }
                ctx->pos+=3;
            } else if (((ctx->end - ctx->pos) > 8) && !strncmp(ctx->pos, "!DOCTYPE", 8)) {
                int embedded_dtd = 0;
                ctx->pos+=8;
                while (ctx->pos < ctx->end) {
                    find_next(ctx, " \t\r\n[>", 6, 1);
                    if (*ctx->pos == '[') {
                        embedded_dtd = 1;
                        break;
                    } else if (*ctx->pos == '>') {
                        /* end of DOCTYPE found already */
                        ctx->pos++;
                        break;
                    } else {
                        parse_skip_ws(ctx);
                    }
                }
                if (embedded_dtd) {
                    find_str(ctx, "]>", 2, 1);
                    if (strncmp(ctx->pos, "]>", 2)) {
                        PLIST_XML_ERR("Couldn't find end of DOCTYPE\n");
                        ctx->pos = ctx->end;
                        return;
                    }
                    ctx->pos += 2;
                }
            } else {
                p = ctx->pos;
                find_next(ctx, " \r\n\t>", 5, 1);
                PLIST_XML_ERR("Invalid special tag <%.*s> encountered\n", (int)(ctx->pos - p), p);
                ctx->err++;
            }
            continue;
        } else {
            int is_empty = 0;
            int closing_tag = 0;
            p = ctx->pos;
            find_next(ctx," \r\n\t<>", 6, 0);
            if (ctx->pos >= ctx->end) {
                PLIST_XML_ERR("Unexpected EOF while parsing XML\n");
                ctx->pos = ctx->end;
                ctx->err++;
                free(keyname);
                return;
            }
            int taglen = ctx->pos - p;
            char *tag = malloc(taglen + 1);
            strncpy(tag, p, taglen);
            tag[taglen] = '\0';
            if (*ctx->pos != '>') {
                find_next(ctx, "<>", 2, 1);
            }
            if (ctx->pos >= ctx->end) {
                PLIST_XML_ERR("Unexpected EOF while parsing XML\n");
                ctx->pos = ctx->end;
                ctx->err++;
                free(tag);
                free(keyname);
                return;
            }
            if (*ctx->pos != '>') {
                PLIST_XML_ERR("Missing '>' for tag <%s\n", tag);
                ctx->pos = ctx->end;
                ctx->err++;
                free(tag);
                free(keyname);
                return;
            }
            if (*(ctx->pos-1) == '/') {
                int idx = ctx->pos - p - 1;
                if (idx < taglen)
                    tag[idx] = '\0';
                is_empty = 1;
            }
            ctx->pos++;
            if (!strcmp(tag, "plist")) {
                free(tag);
                if (is_empty) {
                    PLIST_XML_ERR("Empty plist tag\n");
                    return;
                }
                if (!*plist) {
                    /* only process first plist node found */
                    node_from_xml(ctx, plist, depth+1);
                }
                continue;
            } else if (depth == 1 && !strcmp(tag, "/plist")) {
                if (!*plist) {
                    PLIST_XML_ERR("Empty plist tag\n");
                }
                free(tag);
                free(keyname);
                return;
            } else if (depth == 1 && *plist) {
                PLIST_XML_ERR("Unexpected tag <%s> found while </plist> is expected\n", tag);
                ctx->err++;
                free(tag);
                free(keyname);
                return;
            }

            plist_data_t data = plist_new_plist_data();
            plist_t subnode = plist_new_node(data);

            if (!strcmp(tag, XPLIST_DICT)) {
                data->type = PLIST_DICT;
            } else if (!strcmp(tag, XPLIST_ARRAY)) {
                data->type = PLIST_ARRAY;
            } else if (!strcmp(tag, XPLIST_INT)) {
                if (!is_empty) {
                    text_part_t first_part = { NULL, 0, 0, NULL };
                    text_part_t *tp = get_text_parts(ctx, tag, taglen, 1, &first_part);
                    if (!tp) {
                        PLIST_XML_ERR("Could not parse text content for '%s' node\n", tag);
                        text_parts_free(first_part.next);
                        free(tag);
                        free(keyname);
                        break;
                    }
                    if (tp->begin) {
                        int requires_free = 0;
                        char *str_content = text_parts_get_content(tp, 0, NULL, &requires_free);
                        if (!str_content) {
                            PLIST_XML_ERR("Could not get text content for '%s' node\n", tag);
                            text_parts_free(first_part.next);
                            ctx->err++;
                            free(tag);
                            free(keyname);
                            break;
                        }
                        char *str = str_content;
                        int is_negative = 0;
                        if ((str[0] == '-') || (str[0] == '+')) {
                            if (str[0] == '-') {
                                is_negative = 1;
                            }
                            str++;
                        }
                        data->intval = strtoull((char*)str, NULL, 0);
                        if (is_negative || (data->intval <= INT64_MAX)) {
                            int64_t v = data->intval;
                            if (is_negative) {
                                v = -v;
                            }
                            data->intval = (uint64_t)v;
                            data->length = 8;
                        } else {
                            data->length = 16;
                        }
                        if (requires_free) {
                            free(str_content);
                        }
                    } else {
                        is_empty = 1;
                    }
                    text_parts_free(tp->next);
                }
                if (is_empty) {
                    data->intval = 0;
                    data->length = 8;
                }
                data->type = PLIST_UINT;
            } else if (!strcmp(tag, XPLIST_REAL)) {
                if (!is_empty) {
                    text_part_t first_part = { NULL, 0, 0, NULL };
                    text_part_t *tp = get_text_parts(ctx, tag, taglen, 1, &first_part);
                    if (!tp) {
                        PLIST_XML_ERR("Could not parse text content for '%s' node\n", tag);
                        text_parts_free(first_part.next);
                        free(tag);
                        free(keyname);
                        break;
                    }
                    if (tp->begin) {
                        int requires_free = 0;
                        char *str_content = text_parts_get_content(tp, 0, NULL, &requires_free);
                        if (!str_content) {
                            PLIST_XML_ERR("Could not get text content for '%s' node\n", tag);
                            text_parts_free(first_part.next);
                            ctx->err++;
                            free(tag);
                            free(keyname);
                            break;
                        }
                        data->realval = atof(str_content);
                        if (requires_free) {
                            free(str_content);
                        }
                    }
                    text_parts_free(tp->next);
                }
                data->type = PLIST_REAL;
                data->length = 8;
            } else if (!strcmp(tag, XPLIST_TRUE)) {
                if (!is_empty) {
                    get_text_parts(ctx, tag, taglen, 1, NULL);
                }
                data->type = PLIST_BOOLEAN;
                data->boolval = 1;
                data->length = 1;
            } else if (!strcmp(tag, XPLIST_FALSE)) {
                if (!is_empty) {
                    get_text_parts(ctx, tag, taglen, 1, NULL);
                }
                data->type = PLIST_BOOLEAN;
                data->boolval = 0;
                data->length = 1;
            } else if (!strcmp(tag, XPLIST_STRING) || !strcmp(tag, XPLIST_KEY)) {
                if (!is_empty) {
                    text_part_t first_part = { NULL, 0, 0, NULL };
                    text_part_t *tp = get_text_parts(ctx, tag, taglen, 0, &first_part);
                    char *str = NULL;
                    size_t length = 0;
                    if (!tp) {
                        PLIST_XML_ERR("Could not parse text content for '%s' node\n", tag);
                        text_parts_free(first_part.next);
                        free(tag);
                        free(keyname);
                        break;
                    }
                    str = text_parts_get_content(tp, 1, &length, NULL);
                    text_parts_free(first_part.next);
                    if (!str) {
                        PLIST_XML_ERR("Could not get text content for '%s' node\n", tag);
                        ctx->err++;
                        free(tag);
                        free(keyname);
                        return;
                    }
                    if (!strcmp(tag, "key") && !keyname && *plist && (plist_get_node_type(*plist) == PLIST_DICT)) {
                        keyname = str;
                        free(tag);
                        plist_free(subnode);
                        subnode = NULL;
                        continue;
                    } else {
                        data->strval = str;
                        data->length = length;
                    }
                } else {
                    data->strval = strdup("");
                    data->length = 0;
                }
                data->type = PLIST_STRING;
            } else if (!strcmp(tag, XPLIST_DATA)) {
                if (!is_empty) {
                    text_part_t first_part = { NULL, 0, 0, NULL };
                    text_part_t *tp = get_text_parts(ctx, tag, taglen, 1, &first_part);
                    if (!tp) {
                        PLIST_XML_ERR("Could not parse text content for '%s' node\n", tag);
                        text_parts_free(first_part.next);
                        free(tag);
                        free(keyname);
                        break;
                    }
                    if (tp->begin) {
                        int requires_free = 0;
                        char *str_content = text_parts_get_content(tp, 0, NULL, &requires_free);
                        if (!str_content) {
                            PLIST_XML_ERR("Could not get text content for '%s' node\n", tag);
                            text_parts_free(first_part.next);
                            ctx->err++;
                            free(tag);
                            free(keyname);
                            break;
                        }
                        size_t size = tp->length;
                        data->buff = base64decode(str_content, &size);
                        data->length = size;

                        if (requires_free) {
                            free(str_content);
                        }
                    }
                    text_parts_free(tp->next);
                }
                data->type = PLIST_DATA;
            } else if (!strcmp(tag, XPLIST_DATE)) {
                if (!is_empty) {
                    text_part_t first_part = { NULL, 0, 0, NULL };
                    text_part_t *tp = get_text_parts(ctx, tag, taglen, 1, &first_part);
                    if (!tp) {
                        PLIST_XML_ERR("Could not parse text content for '%s' node\n", tag);
                        text_parts_free(first_part.next);
                        free(tag);
                        free(keyname);
                        break;
                    }
                    Time64_T timev = 0;
                    if (tp->begin) {
                        int requires_free = 0;
                        size_t length = 0;
                        char *str_content = text_parts_get_content(tp, 0, &length, &requires_free);
                        if (!str_content) {
                            PLIST_XML_ERR("Could not get text content for '%s' node\n", tag);
                            text_parts_free(first_part.next);
                            ctx->err++;
                            free(tag);
                            free(keyname);
                            break;
                        }

                        if ((length >= 11) && (length < 32)) {
                            /* we need to copy here and 0-terminate because sscanf will read the entire string (whole rest of XML data) which can be huge */
                            char strval[32];
                            struct TM btime;
                            strncpy(strval, str_content, length);
                            strval[tp->length] = '\0';
                            parse_date(strval, &btime);
                            timev = timegm64(&btime);
                        } else {
                            PLIST_XML_ERR("Invalid text content in date node\n");
                        }
                        if (requires_free) {
                            free(str_content);
                        }
                    }
                    text_parts_free(tp->next);
                    data->realval = (double)(timev - MAC_EPOCH);
                }
                data->length = sizeof(double);
                data->type = PLIST_DATE;
            } else if (tag[0] == '/') {
                 closing_tag = 1;
            } else {
                PLIST_XML_ERR("Unexpected tag <%s%s> encountered\n", tag, (is_empty) ? "/" : "");
                ctx->pos = ctx->end;
                ctx->err++;
                free(tag);
                free(keyname);
                return;
            }
            if (subnode && !closing_tag) {
                /* parse sub nodes for structured types */
                if (data->type == PLIST_DICT || data->type == PLIST_ARRAY) {
                    if (!is_empty) {
                        /* only if not empty */
                        node_from_xml(ctx, &subnode, depth+1);
                        if (ctx->err) {
                            /* make sure to bail out if parsing failed */
                            free(tag);
                            free(keyname);
                            return;
                        }
                        if ((data->type == PLIST_DICT) && (plist_dict_get_size(subnode) == 1)) {
                            /* convert XML CF$UID dictionaries to PLIST_UID nodes */
                            plist_t uid = plist_dict_get_item(subnode, "CF$UID");
                            if (uid) {
                                uint64_t val = 0;
                                plist_get_uint_val(uid, &val);
                                plist_dict_remove_item(subnode, "CF$UID");
                                plist_data_t nodedata = plist_get_data((node_t*)subnode);
                                free(nodedata->buff);
                                nodedata->type = PLIST_UID;
                                nodedata->length = sizeof(uint64_t);
                                nodedata->intval = val;
                            }
                        }
                    }
                }
                if (!*plist) {
                    /* no parent? make this node the new parent node */
                    *plist = subnode;
                    subnode = NULL;
                } else {
                    switch (plist_get_node_type(*plist)) {
                    case PLIST_DICT:
                        if (!keyname) {
                            PLIST_XML_ERR("missing key name while adding dict item\n");
                            ctx->err++;
                            break;
                        }
                        plist_dict_set_item(*plist, keyname, subnode);
                        subnode = NULL;
                        break;
                    case PLIST_ARRAY:
                        plist_array_append_item(*plist, subnode);
                        subnode = NULL;
                        break;
                    default:
                        /* should not happen */
                        PLIST_XML_ERR("while parsing XML plist: parent is not a structered node.\n");
                        ctx->err++;
                        break;
                    }
                }
            } else if (closing_tag && *plist) {
                switch (plist_get_node_type(*plist)) {
                case PLIST_DICT:
                    if (keyname) {
                        PLIST_XML_ERR("missing value node in dict\n");
                        ctx->err++;
                    } else if (strcmp(tag+1, XPLIST_DICT) != 0) {
                        PLIST_XML_ERR("closing tag mismatch, expected: </%s> found: <%s>\n", XPLIST_DICT, tag);
                        ctx->err++;
                    }
                    break;
                case PLIST_ARRAY:
                    if (strcmp(tag+1, XPLIST_ARRAY) != 0) {
                        PLIST_XML_ERR("closing tag mismatch, expected: </%s> found: <%s>\n", XPLIST_ARRAY, tag);
                        ctx->err++;
                    }
                    break;
                default:
                    /* should not happen */
                    PLIST_XML_ERR("expected structered node but got type %d\n", plist_get_node_type(*plist));
                    ctx->err++;
                    break;
                }
            }
            free(tag);
            free(keyname);
            keyname = NULL;
            plist_free(subnode);
            if (closing_tag) {
                break;
            }
        }
    }
    if (depth == 1) {
        PLIST_XML_ERR("EOF while </plist> tag is expected\n");
        ctx->err++;
    }
    if (ctx->err) {
        plist_free(*plist);
        *plist = NULL;
    }
}

PLIST_API void plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist)
{
    if (!plist_xml || (length == 0)) {
        *plist = NULL;
        return;
    }

    struct _parse_ctx ctx = { plist_xml, plist_xml + length, 0 };

    node_from_xml(&ctx, plist, 0);
}
