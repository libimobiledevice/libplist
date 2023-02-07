/*
 * oplist.c
 * OpenStep plist implementation
 *
 * Copyright (c) 2021-2022 Nikias Bassen, All Rights Reserved.
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <inttypes.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include <node.h>

#include "plist.h"
#include "strbuf.h"

#ifdef DEBUG
static int plist_ostep_debug = 0;
#define PLIST_OSTEP_ERR(...) if (plist_ostep_debug) { fprintf(stderr, "libplist[ostepparser] ERROR: " __VA_ARGS__); }
#define PLIST_OSTEP_WRITE_ERR(...) if (plist_ostep_debug) { fprintf(stderr, "libplist[ostepwriter] ERROR: " __VA_ARGS__); }
#else
#define PLIST_OSTEP_ERR(...)
#define PLIST_OSTEP_WRITE_ERR(...)
#endif

void plist_ostep_init(void)
{
    /* init OpenStep stuff */
#ifdef DEBUG
    char *env_debug = getenv("PLIST_OSTEP_DEBUG");
    if (env_debug && !strcmp(env_debug, "1")) {
        plist_ostep_debug = 1;
    }
#endif
}

void plist_ostep_deinit(void)
{
    /* deinit OpenStep plist stuff */
}

void plist_ostep_set_debug(int debug)
{
#if DEBUG
    plist_ostep_debug = debug;
#endif
}

#ifndef HAVE_STRNDUP
static char* strndup(const char* str, size_t len)
{
    char *newstr = (char *)malloc(len+1);
    if (newstr) {
        strncpy(newstr, str, len);
        newstr[len]= '\0';
    }
    return newstr;
}
#endif

static size_t dtostr(char *buf, size_t bufsize, double realval)
{
    size_t len = 0;
    if (isnan(realval)) {
        len = snprintf(buf, bufsize, "nan");
    } else if (isinf(realval)) {
        len = snprintf(buf, bufsize, "%cinfinity", (realval > 0.0) ? '+' : '-');
    } else if (realval == 0.0f) {
        len = snprintf(buf, bufsize, "0.0");
    } else {
        size_t i = 0;
        len = snprintf(buf, bufsize, "%.*g", 17, realval);
        for (i = 0; buf && i < len; i++) {
            if (buf[i] == ',') {
                buf[i] = '.';
                break;
            } else if (buf[i] == '.') {
                break;
            }
        }
    }
    return len;
}

static const char allowed_unquoted_chars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static int str_needs_quotes(const char* str, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++) {
        if (!allowed_unquoted_chars[(unsigned char)str[i]]) {
            return 1;
        }
    }
    return 0;
}

static int node_to_openstep(node_t node, bytearray_t **outbuf, uint32_t depth, int prettify)
{
    plist_data_t node_data = NULL;

    char *val = NULL;
    size_t val_len = 0;

    uint32_t i = 0;

    if (!node)
        return PLIST_ERR_INVALID_ARG;

    node_data = plist_get_data(node);

    switch (node_data->type)
    {
    case PLIST_INT:
        val = (char*)malloc(64);
        if (node_data->length == 16) {
            val_len = snprintf(val, 64, "%"PRIu64, node_data->intval);
        } else {
            val_len = snprintf(val, 64, "%"PRIi64, node_data->intval);
        }
        str_buf_append(*outbuf, val, val_len);
        free(val);
        break;

    case PLIST_REAL:
        val = (char*)malloc(64);
        val_len = dtostr(val, 64, node_data->realval);
        str_buf_append(*outbuf, val, val_len);
        free(val);
        break;

    case PLIST_STRING:
    case PLIST_KEY: {
        const char *charmap[32] = {
            "\\U0000", "\\U0001", "\\U0002", "\\U0003", "\\U0004", "\\U0005", "\\U0006", "\\U0007",
            "\\b",     "\\t",     "\\n",     "\\U000b", "\\f",     "\\r",     "\\U000e", "\\U000f",
            "\\U0010", "\\U0011", "\\U0012", "\\U0013", "\\U0014", "\\U0015", "\\U0016", "\\U0017",
            "\\U0018", "\\U0019", "\\U001a", "\\U001b", "\\U001c", "\\U001d", "\\U001e", "\\U001f",
        };
        size_t j = 0;
        size_t len = 0;
        off_t start = 0;
        off_t cur = 0;
        int needs_quotes;

        len = node_data->length;

        needs_quotes = str_needs_quotes(node_data->strval, len);

        if (needs_quotes) {
            str_buf_append(*outbuf, "\"", 1);
        }

        for (j = 0; j < len; j++) {
            unsigned char ch = (unsigned char)node_data->strval[j];
            if (ch < 0x20) {
                str_buf_append(*outbuf, node_data->strval + start, cur - start);
                str_buf_append(*outbuf, charmap[ch], (charmap[ch][1] == 'u') ? 6 : 2);
                start = cur+1;
            } else if (ch == '"') {
                str_buf_append(*outbuf, node_data->strval + start, cur - start);
                str_buf_append(*outbuf, "\\\"", 2);
                start = cur+1;
            }
            cur++;
        }
        str_buf_append(*outbuf, node_data->strval + start, cur - start);

        if (needs_quotes) {
            str_buf_append(*outbuf, "\"", 1);
        }

        } break;

    case PLIST_ARRAY: {
        str_buf_append(*outbuf, "(", 1);
        node_t ch;
        uint32_t cnt = 0;
        for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
            if (cnt > 0) {
                str_buf_append(*outbuf, ",", 1);
            }
            if (prettify) {
                str_buf_append(*outbuf, "\n", 1);
                for (i = 0; i <= depth; i++) {
                    str_buf_append(*outbuf, "  ", 2);
                }
            }
            int res = node_to_openstep(ch, outbuf, depth+1, prettify);
            if (res < 0) {
                return res;
            }
            cnt++;
        }
        if (cnt > 0 && prettify) {
            str_buf_append(*outbuf, "\n", 1);
            for (i = 0; i < depth; i++) {
                str_buf_append(*outbuf, "  ", 2);
            }
        }
        str_buf_append(*outbuf, ")", 1);
        } break;
    case PLIST_DICT: {
        str_buf_append(*outbuf, "{", 1);
        node_t ch;
        uint32_t cnt = 0;
        for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
            if (cnt > 0 && cnt % 2 == 0) {
                str_buf_append(*outbuf, ";", 1);
            }
            if (cnt % 2 == 0 && prettify) {
                str_buf_append(*outbuf, "\n", 1);
                for (i = 0; i <= depth; i++) {
                    str_buf_append(*outbuf, "  ", 2);
                }
            }
            int res = node_to_openstep(ch, outbuf, depth+1, prettify);
            if (res < 0) {
                return res;
            }
            if (cnt % 2 == 0) {
                if (prettify) {
                  str_buf_append(*outbuf, " = ", 3);
                } else {
                  str_buf_append(*outbuf, "=", 1);
                }
            }
            cnt++;
        }
        if (cnt > 0) {
          str_buf_append(*outbuf, ";", 1);
        }
        if (cnt > 0 && prettify) {
            str_buf_append(*outbuf, "\n", 1);
            for (i = 0; i < depth; i++) {
                str_buf_append(*outbuf, "  ", 2);
            }
        }
        str_buf_append(*outbuf, "}", 1);
        } break;
    case PLIST_DATA: {
        size_t j = 0;
        size_t len = 0;
        str_buf_append(*outbuf, "<", 1);
        len = node_data->length;
        for (j = 0; j < len; j++) {
            char charb[4];
            if (prettify && j > 0 && (j % 4 == 0))
              str_buf_append(*outbuf, " ", 1);
            sprintf(charb, "%02x", (unsigned char)node_data->buff[j]);
            str_buf_append(*outbuf, charb, 2);
        }
        str_buf_append(*outbuf, ">", 1);
        } break;
    case PLIST_BOOLEAN:
        PLIST_OSTEP_WRITE_ERR("PLIST_BOOLEAN type is not valid for OpenStep format\n");
        return PLIST_ERR_FORMAT;
    case PLIST_NULL:
        PLIST_OSTEP_WRITE_ERR("PLIST_NULL type is not valid for OpenStep format\n");
        return PLIST_ERR_FORMAT;
    case PLIST_DATE:
        // NOT VALID FOR OPENSTEP
        PLIST_OSTEP_WRITE_ERR("PLIST_DATE type is not valid for OpenStep format\n");
        return PLIST_ERR_FORMAT;
    case PLIST_UID:
        // NOT VALID FOR OPENSTEP
        PLIST_OSTEP_WRITE_ERR("PLIST_UID type is not valid for OpenStep format\n");
        return PLIST_ERR_FORMAT;
    default:
        return PLIST_ERR_UNKNOWN;
    }

    return PLIST_ERR_SUCCESS;
}

#define PO10i_LIMIT (INT64_MAX/10)

/* based on https://stackoverflow.com/a/4143288 */
static int num_digits_i(int64_t i)
{
    int n;
    int64_t po10;
    n=1;
    if (i < 0) {
        i = (i == INT64_MIN) ? INT64_MAX : -i;
        n++;
    }
    po10=10;
    while (i>=po10) {
        n++;
        if (po10 > PO10i_LIMIT) break;
        po10*=10;
    }
    return n;
}

#define PO10u_LIMIT (UINT64_MAX/10)

/* based on https://stackoverflow.com/a/4143288 */
static int num_digits_u(uint64_t i)
{
    int n;
    uint64_t po10;
    n=1;
    po10=10;
    while (i>=po10) {
        n++;
        if (po10 > PO10u_LIMIT) break;
        po10*=10;
    }
    return n;
}

static int node_estimate_size(node_t node, uint64_t *size, uint32_t depth, int prettify)
{
    plist_data_t data;
    if (!node) {
        return PLIST_ERR_INVALID_ARG;
    }
    data = plist_get_data(node);
    if (node->children) {
        node_t ch;
        unsigned int n_children = node_n_children(node);
        for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
            int res = node_estimate_size(ch, size, depth + 1, prettify);
            if (res < 0) {
                return res;
            }
        }
        switch (data->type) {
        case PLIST_DICT:
            *size += 2; // '{' and '}'
            *size += n_children; // number of '=' and ';'
            if (prettify) {
                *size += n_children*2; // number of '\n' and extra spaces
                *size += (uint64_t)n_children * (depth+1); // indent for every 2nd child
                *size += 1; // additional '\n'
            }
            break;
        case PLIST_ARRAY:
            *size += 2; // '(' and ')'
            *size += n_children-1; // number of ','
            if (prettify) {
                *size += n_children; // number of '\n'
                *size += (uint64_t)n_children * ((depth+1)<<1); // indent for every child
                *size += 1; // additional '\n'
            }
            break;
        default:
            break;
	}
        if (prettify)
            *size += (depth << 1); // indent for {} and ()
    } else {
        switch (data->type) {
        case PLIST_STRING:
        case PLIST_KEY:
            *size += data->length;
            *size += 2;
            break;
        case PLIST_INT:
            if (data->length == 16) {
                *size += num_digits_u(data->intval);
            } else {
                *size += num_digits_i((int64_t)data->intval);
            }
            break;
        case PLIST_REAL:
            *size += dtostr(NULL, 0, data->realval);
            break;
        case PLIST_DICT:
        case PLIST_ARRAY:
            *size += 2;
            break;
        case PLIST_DATA:
            *size += 2; // < and >
            *size += data->length*2;
            if (prettify)
                *size += data->length/4;
            break;
        case PLIST_BOOLEAN:
            // NOT VALID FOR OPENSTEP
            PLIST_OSTEP_WRITE_ERR("PLIST_BOOLEAN type is not valid for OpenStep format\n");
            return PLIST_ERR_FORMAT;
        case PLIST_DATE:
            // NOT VALID FOR OPENSTEP
            PLIST_OSTEP_WRITE_ERR("PLIST_DATE type is not valid for OpenStep format\n");
            return PLIST_ERR_FORMAT;
        case PLIST_UID:
            // NOT VALID FOR OPENSTEP
            PLIST_OSTEP_WRITE_ERR("PLIST_UID type is not valid for OpenStep format\n");
            return PLIST_ERR_FORMAT;
        default:
            PLIST_OSTEP_WRITE_ERR("invalid node type encountered\n");
            return PLIST_ERR_UNKNOWN;
        }
    }
    return PLIST_ERR_SUCCESS;
}

PLIST_API int plist_to_openstep(plist_t plist, char **openstep, uint32_t* length, int prettify)
{
    uint64_t size = 0;
    int res;

    if (!plist || !openstep || !length) {
        return PLIST_ERR_INVALID_ARG;
    }

    res = node_estimate_size(plist, &size, 0, prettify);
    if (res < 0) {
        return res;
    }

    strbuf_t *outbuf = str_buf_new(size);
    if (!outbuf) {
        PLIST_OSTEP_WRITE_ERR("Could not allocate output buffer");
        return PLIST_ERR_NO_MEM;
    }

    res = node_to_openstep(plist, &outbuf, 0, prettify);
    if (res < 0) {
        str_buf_free(outbuf);
        *openstep = NULL;
        *length = 0;
        return res;
    }
    if (prettify) {
        str_buf_append(outbuf, "\n", 1);
    }

    str_buf_append(outbuf, "\0", 1);

    *openstep = outbuf->data;
    *length = outbuf->len - 1;

    outbuf->data = NULL;
    str_buf_free(outbuf);

    return PLIST_ERR_SUCCESS;
}

struct _parse_ctx {
    const char *start;
    const char *pos;
    const char *end;
    int err;
    uint32_t depth;
};
typedef struct _parse_ctx* parse_ctx;

static void parse_skip_ws(parse_ctx ctx)
{
    while (ctx->pos < ctx->end) {
        // skip comments
        if (*ctx->pos == '/' && (ctx->end - ctx->pos > 1)) {
            if (*(ctx->pos+1) == '/') {
                ctx->pos++;
                while (ctx->pos < ctx->end) {
                    if ((*ctx->pos == '\n') || (*ctx->pos == '\r')) {
                        break;
                    }
                    ctx->pos++;
                }
            } else if (*(ctx->pos+1) == '*') {
                ctx->pos++;
                while (ctx->pos < ctx->end) {
                    if (*ctx->pos == '*' && (ctx->end - ctx->pos > 1)) {
                        if (*(ctx->pos+1) == '/') {
                            ctx->pos+=2;
                            break;
                        }
                    }
                    ctx->pos++;
                }
            }
            if (ctx->pos >= ctx->end) {
                break;
            }
        }
        // break on any char that's not white space
        if (!(((*(ctx->pos) == ' ') || (*(ctx->pos) == '\t') || (*(ctx->pos) == '\r') || (*(ctx->pos) == '\n')))) {
            break;
        }
        ctx->pos++;
    }
}

#define HEX_DIGIT(x) ((x <= '9') ? (x - '0') : ((x <= 'F') ? (x - 'A' + 10) : (x - 'a' + 10)))

static int node_from_openstep(parse_ctx ctx, plist_t *plist);

static void parse_dict_data(parse_ctx ctx, plist_t dict)
{
    plist_t key = NULL;
    plist_t val = NULL;
    while (ctx->pos < ctx->end && !ctx->err) {
        parse_skip_ws(ctx);
        if (ctx->pos >= ctx->end || *ctx->pos == '}') {
            break;
        }
        key = NULL;
        ctx->err = node_from_openstep(ctx, &key);
        if (ctx->err != 0) {
            break;
        }
        if (!PLIST_IS_STRING(key)) {
            PLIST_OSTEP_ERR("Invalid type for dictionary key at offset %ld\n", ctx->pos - ctx->start);
            ctx->err++;
            break;
        }
        parse_skip_ws(ctx);
        if (ctx->pos >= ctx->end) {
            PLIST_OSTEP_ERR("EOF while parsing dictionary '=' delimiter at offset %ld\n", ctx->pos - ctx->start);
            ctx->err++;
            break;
        }
        if (*ctx->pos != '=') {
            PLIST_OSTEP_ERR("Missing '=' while parsing dictionary item at offset %ld\n", ctx->pos - ctx->start);
            ctx->err++;
            break;
        }
        ctx->pos++;
        if (ctx->pos >= ctx->end) {
            PLIST_OSTEP_ERR("EOF while parsing dictionary item at offset %ld\n", ctx->pos - ctx->start);
            ctx->err++;
            break;
        }
        val = NULL;
        ctx->err = node_from_openstep(ctx, &val);
        if (ctx->err != 0) {
            break;
        }
        if (!val) {
            PLIST_OSTEP_ERR("Missing value for dictionary item at offset %ld\n", ctx->pos - ctx->start);
            ctx->err++;
            break;
        }
        parse_skip_ws(ctx);
        if (ctx->pos >= ctx->end) {
            PLIST_OSTEP_ERR("EOF while parsing dictionary item terminator ';' at offset %ld\n", ctx->pos - ctx->start);
            ctx->err++;
            break;
        }
        if (*ctx->pos != ';') {
            PLIST_OSTEP_ERR("Missing terminating ';' while parsing dictionary item at offset %ld\n", ctx->pos - ctx->start);
            ctx->err++;
            break;
        }

        plist_dict_set_item(dict, plist_get_string_ptr(key, NULL), val);
        plist_free(key);
        key = NULL;
        val = NULL;

        ctx->pos++;
    }
    plist_free(key);
    plist_free(val);
}

static int node_from_openstep(parse_ctx ctx, plist_t *plist)
{
    plist_t subnode = NULL;
    const char *p = NULL;
    ctx->depth++;
    if (ctx->depth > 1000) {
        PLIST_OSTEP_ERR("Too many levels of recursion (%u) at offset %ld\n", ctx->depth, ctx->pos - ctx->start);
        ctx->err++;
        return PLIST_ERR_PARSE;
    }
    while (ctx->pos < ctx->end && !ctx->err) {
        parse_skip_ws(ctx);
        if (ctx->pos >= ctx->end) {
            break;
        }
        plist_data_t data = plist_new_plist_data();
        if (*ctx->pos == '{') {
            data->type = PLIST_DICT;
            subnode = plist_new_node(data);
            ctx->pos++;
            parse_dict_data(ctx, subnode);
            if (ctx->err) {
                goto err_out;
            }
            if (ctx->pos >= ctx->end) {
                PLIST_OSTEP_ERR("EOF while parsing dictionary terminator '}' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                break;
            }
            if (*ctx->pos != '}') {
                PLIST_OSTEP_ERR("Missing terminating '}' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            ctx->pos++;
            *plist = subnode;
            parse_skip_ws(ctx);
            break;
        } else if (*ctx->pos == '(') {
            data->type = PLIST_ARRAY;
            subnode = plist_new_node(data);
            ctx->pos++;
            plist_t tmp = NULL;
            while (ctx->pos < ctx->end && !ctx->err) {
                parse_skip_ws(ctx);
                if (ctx->pos >= ctx->end || *ctx->pos == ')') {
                    break;
                }
                ctx->err = node_from_openstep(ctx, &tmp);
                if (ctx->err != 0) {
                    break;
                }
                if (!tmp) {
                    ctx->err++;
                    break;
                }
                plist_array_append_item(subnode, tmp);
                tmp = NULL;
                parse_skip_ws(ctx);
                if (ctx->pos >= ctx->end) {
                    PLIST_OSTEP_ERR("EOF while parsing array item delimiter ',' at offset %ld\n", ctx->pos - ctx->start);
                    ctx->err++;
                    break;
                }
                if (*ctx->pos != ',') {
                    break;
                }
                ctx->pos++;
            }
	    plist_free(tmp);
	    tmp = NULL;
            if (ctx->err) {
                goto err_out;
            }
            if (ctx->pos >= ctx->end) {
                PLIST_OSTEP_ERR("EOF while parsing array terminator ')' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                break;
            }
            if (*ctx->pos != ')') {
                PLIST_OSTEP_ERR("Missing terminating ')' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            ctx->pos++;
            *plist = subnode;
            parse_skip_ws(ctx);
            break;
        } else if (*ctx->pos == '<') {
            data->type = PLIST_DATA;
            ctx->pos++;
            bytearray_t *bytes = byte_array_new(256);
            while (ctx->pos < ctx->end && !ctx->err) {
                parse_skip_ws(ctx);
                if (ctx->pos >= ctx->end) {
                    PLIST_OSTEP_ERR("EOF while parsing data terminator '>' at offset %ld\n", ctx->pos - ctx->start);
                    ctx->err++;
                    break;
                }
                if (*ctx->pos == '>') {
                    break;
                }
                if (!isxdigit(*ctx->pos)) {
                    PLIST_OSTEP_ERR("Invalid byte group in data at offset %ld\n", ctx->pos - ctx->start);
                    ctx->err++;
                    break;
                }
                uint8_t b = HEX_DIGIT(*ctx->pos);
                ctx->pos++;
                if (ctx->pos >= ctx->end) {
                    PLIST_OSTEP_ERR("Unexpected end of data at offset %ld\n", ctx->pos - ctx->start);
                    ctx->err++;
                    break;
                }
                if (!isxdigit(*ctx->pos)) {
                    PLIST_OSTEP_ERR("Invalid byte group in data at offset %ld\n", ctx->pos - ctx->start);
                    ctx->err++;
                    break;
                }
                b = (b << 4) + HEX_DIGIT(*ctx->pos);
                byte_array_append(bytes, &b, 1);
                ctx->pos++;
            }
            if (ctx->err) {
                byte_array_free(bytes);
                plist_free_data(data);
                goto err_out;
            }
            if (ctx->pos >= ctx->end) {
                byte_array_free(bytes);
                plist_free_data(data);
                PLIST_OSTEP_ERR("EOF while parsing data terminator '>' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            if (*ctx->pos != '>') {
                byte_array_free(bytes);
                plist_free_data(data);
                PLIST_OSTEP_ERR("Missing terminating '>' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            ctx->pos++;
            data->buff = bytes->data;
            data->length = bytes->len;
            bytes->data = NULL;
            byte_array_free(bytes);
            *plist = plist_new_node(data);
            parse_skip_ws(ctx);
            break;
        } else if (*ctx->pos == '"' || *ctx->pos == '\'') {
            char c = *ctx->pos;
            ctx->pos++;
            p = ctx->pos;
            int num_escapes = 0;
            while (ctx->pos < ctx->end) {
                if (*ctx->pos == '\\') {
                    num_escapes++;
                }
                if ((*ctx->pos == c) && (*(ctx->pos-1) != '\\')) {
                    break;
                }
                ctx->pos++;
            }
            if (ctx->pos >= ctx->end) {
                plist_free_data(data);
                PLIST_OSTEP_ERR("EOF while parsing quoted string at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            if (*ctx->pos != c) {
                plist_free_data(data);
                PLIST_OSTEP_ERR("Missing closing quote (%c) at offset %ld\n", c, ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            size_t slen = ctx->pos - p;
            ctx->pos++; // skip the closing quote
            char* strbuf = malloc(slen+1);
            if (num_escapes > 0) {
                size_t i = 0;
                size_t o = 0;
                while (i < slen) {
                    if (p[i] == '\\') {
                        /* handle escape sequence */
                        i++;
                        switch (p[i]) {
                            case '0':
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                            case '6':
                            case '7': {
                                // max 3 digits octal
                                unsigned char chr = 0;
                                int maxd = 3;
                                while ((i < slen) && (p[i] >= '0' && p[i] <= '7') && --maxd) {
                                    chr = (chr << 3) + p[i] - '0';
                                    i++;
                                }
                                strbuf[o++] = (char)chr;
                            }   break;
                            case 'U': {
                                i++;
                                // max 4 digits hex
                                uint16_t wchr = 0;
                                int maxd = 4;
                                while ((i < slen) && isxdigit(p[i]) && maxd--) {
                                    wchr = (wchr << 4) + ((p[i] <= '9') ? (p[i] - '0') : ((p[i] <= 'F') ? (p[i] - 'A' + 10) : (p[i] - 'a' + 10)));
                                    i++;
                                }
                                if (wchr >= 0x800) {
                                    strbuf[o++] = (char)(0xE0 + ((wchr >> 12) & 0xF));
                                    strbuf[o++] = (char)(0x80 + ((wchr >> 6) & 0x3F));
                                    strbuf[o++] = (char)(0x80 + (wchr & 0x3F));
                                } else if (wchr >= 0x80) {
                                    strbuf[o++] = (char)(0xC0 + ((wchr >> 6) & 0x1F));
                                    strbuf[o++] = (char)(0x80 + (wchr & 0x3F));
                                } else {
                                    strbuf[o++] = (char)(wchr & 0x7F);
                                }
                            }   break;
                            case 'a': strbuf[o++] = '\a'; i++; break;
                            case 'b': strbuf[o++] = '\b'; i++; break;
                            case 'f': strbuf[o++] = '\f'; i++; break;
                            case 'n': strbuf[o++] = '\n'; i++; break;
                            case 'r': strbuf[o++] = '\r'; i++; break;
                            case 't': strbuf[o++] = '\t'; i++; break;
                            case 'v': strbuf[o++] = '\v'; i++; break;
                            case '"': strbuf[o++] = '"';  i++; break;
                            case '\'': strbuf[o++] = '\''; i++; break;
                            default:
                                break;
                        }
                    } else {
                        strbuf[o++] = p[i++];
                    }
                }
                strbuf[o] = '\0';
                slen = o;
            } else {
                strncpy(strbuf, p, slen);
                strbuf[slen] = '\0';
            }
            data->type = PLIST_STRING;
            data->strval = strbuf;
            data->length = slen;
            *plist = plist_new_node(data);
            parse_skip_ws(ctx);
            break;
        } else {
            // unquoted string
            size_t slen = 0;
            parse_skip_ws(ctx);
            p = ctx->pos;
            while (ctx->pos < ctx->end) {
                if (!allowed_unquoted_chars[(uint8_t)*ctx->pos]) {
                    break;
                }
                ctx->pos++;
            }
            slen = ctx->pos-p;
            if (slen > 0) {
                data->type = PLIST_STRING;
                data->strval = strndup(p, slen);
                data->length = slen;
                *plist = plist_new_node(data);
                parse_skip_ws(ctx);
                break;
            } else {
                plist_free_data(data);
                PLIST_OSTEP_ERR("Unexpected character when parsing unquoted string at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                break;
            }
        }
        ctx->pos++;
    }
    ctx->depth--;

err_out:
    if (ctx->err) {
        plist_free(subnode);
        plist_free(*plist);
        *plist = NULL;
        return PLIST_ERR_PARSE;
    }
    return PLIST_ERR_SUCCESS;
}

PLIST_API int plist_from_openstep(const char *plist_ostep, uint32_t length, plist_t * plist)
{
    if (!plist) {
        return PLIST_ERR_INVALID_ARG;
    }
    *plist = NULL;
    if (!plist_ostep || (length == 0)) {
        return PLIST_ERR_INVALID_ARG;
    }

    struct _parse_ctx ctx = { plist_ostep, plist_ostep, plist_ostep + length, 0 , 0 };

    int err = node_from_openstep(&ctx, plist);
    if (err == 0) {
        if (!*plist) {
            /* whitespace only file is considered an empty dictionary */
            *plist = plist_new_dict();
        } else if (ctx.pos < ctx.end && *ctx.pos == '=') {
            /* attempt to parse this as 'strings' data */
            plist_free(*plist);
            *plist = NULL;
            plist_t pl = plist_new_dict();
            ctx.pos = plist_ostep;
            parse_dict_data(&ctx, pl);
            if (ctx.err > 0) {
                plist_free(pl);
                PLIST_OSTEP_ERR("Failed to parse strings data\n");
                err = PLIST_ERR_PARSE;
            } else {
                *plist = pl;
            }
        }
    }

    return err;
}
