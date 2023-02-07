/*
 * jplist.c
 * JSON plist implementation
 *
 * Copyright (c) 2019-2021 Nikias Bassen All Rights Reserved.
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
#include "jsmn.h"

#ifdef DEBUG
static int plist_json_debug = 0;
#define PLIST_JSON_ERR(...) if (plist_json_debug) { fprintf(stderr, "libplist[jsonparser] ERROR: " __VA_ARGS__); }
#define PLIST_JSON_WRITE_ERR(...) if (plist_json_debug) { fprintf(stderr, "libplist[jsonwriter] ERROR: " __VA_ARGS__); }
#else
#define PLIST_JSON_ERR(...)
#define PLIST_JSON_WRITE_ERR(...)
#endif

void plist_json_init(void)
{
    /* init JSON stuff */
#ifdef DEBUG
    char *env_debug = getenv("PLIST_JSON_DEBUG");
    if (env_debug && !strcmp(env_debug, "1")) {
        plist_json_debug = 1;
    }
#endif
}

void plist_json_deinit(void)
{
    /* deinit JSON stuff */
}

void plist_json_set_debug(int debug)
{
#ifdef DEBUG
    plist_json_debug = debug;
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

static int node_to_json(node_t node, bytearray_t **outbuf, uint32_t depth, int prettify)
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
    case PLIST_BOOLEAN:
    {
        if (node_data->boolval) {
            str_buf_append(*outbuf, "true", 4);
        } else {
            str_buf_append(*outbuf, "false", 5);
        }
    }
    break;

    case PLIST_NULL:
        str_buf_append(*outbuf, "null", 4);
	break;

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
            "\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007",
            "\\b",     "\\t",     "\\n",     "\\u000b", "\\f",     "\\r",     "\\u000e", "\\u000f",
            "\\u0010", "\\u0011", "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017",
            "\\u0018", "\\u0019", "\\u001a", "\\u001b", "\\u001c", "\\u001d", "\\u001e", "\\u001f",
        };
        size_t j = 0;
        size_t len = 0;
        off_t start = 0;
        off_t cur = 0;

        str_buf_append(*outbuf, "\"", 1);

        len = node_data->length;
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

        str_buf_append(*outbuf, "\"", 1);
        } break;

    case PLIST_ARRAY: {
        str_buf_append(*outbuf, "[", 1);
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
            int res = node_to_json(ch, outbuf, depth+1, prettify);
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
        str_buf_append(*outbuf, "]", 1);
        } break;
    case PLIST_DICT: {
        str_buf_append(*outbuf, "{", 1);
        node_t ch;
        uint32_t cnt = 0;
        for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
            if (cnt > 0 && cnt % 2 == 0) {
                str_buf_append(*outbuf, ",", 1);
            }
            if (cnt % 2 == 0 && prettify) {
                str_buf_append(*outbuf, "\n", 1);
                for (i = 0; i <= depth; i++) {
                    str_buf_append(*outbuf, "  ", 2);
                }
            }
            int res = node_to_json(ch, outbuf, depth+1, prettify);
            if (res < 0) {
                return res;
            }
            if (cnt % 2 == 0) {
                str_buf_append(*outbuf, ":", 1);
                if (prettify) {
                  str_buf_append(*outbuf, " ", 1);
                }
            }
            cnt++;
        }
        if (cnt > 0 && prettify) {
            str_buf_append(*outbuf, "\n", 1);
            for (i = 0; i < depth; i++) {
                str_buf_append(*outbuf, "  ", 2);
            }
        }
        str_buf_append(*outbuf, "}", 1);
        } break;
    case PLIST_DATA:
        // NOT VALID FOR JSON
        PLIST_JSON_WRITE_ERR("PLIST_DATA type is not valid for JSON format\n");
        return PLIST_ERR_FORMAT;
    case PLIST_DATE:
        // NOT VALID FOR JSON
        PLIST_JSON_WRITE_ERR("PLIST_DATE type is not valid for JSON format\n");
        return PLIST_ERR_FORMAT;
    case PLIST_UID:
        // NOT VALID FOR JSON
        PLIST_JSON_WRITE_ERR("PLIST_UID type is not valid for JSON format\n");
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
            *size += n_children-1; // number of ':' and ','
            if (prettify) {
                *size += n_children; // number of '\n' and extra space
                *size += (uint64_t)n_children * (depth+1); // indent for every 2nd child
                *size += 1; // additional '\n'
            }
            break;
        case PLIST_ARRAY:
            *size += 2; // '[' and ']'
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
            *size += (depth << 1); // indent for {} and []
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
        case PLIST_BOOLEAN:
            *size += ((data->boolval) ? 4 : 5);
            break;
        case PLIST_NULL:
            *size += 4;
            break;
        case PLIST_DICT:
        case PLIST_ARRAY:
            *size += 2;
            break;
        case PLIST_DATA:
            // NOT VALID FOR JSON
            PLIST_JSON_WRITE_ERR("PLIST_DATA type is not valid for JSON format\n");
            return PLIST_ERR_FORMAT;
        case PLIST_DATE:
            // NOT VALID FOR JSON
            PLIST_JSON_WRITE_ERR("PLIST_DATE type is not valid for JSON format\n");
            return PLIST_ERR_FORMAT;
        case PLIST_UID:
            // NOT VALID FOR JSON
            PLIST_JSON_WRITE_ERR("PLIST_UID type is not valid for JSON format\n");
            return PLIST_ERR_FORMAT;
        default:
            PLIST_JSON_WRITE_ERR("invalid node type encountered\n");
            return PLIST_ERR_UNKNOWN;
        }
    }
    return PLIST_ERR_SUCCESS;
}

PLIST_API int plist_to_json(plist_t plist, char **json, uint32_t* length, int prettify)
{
    uint64_t size = 0;
    int res;

    if (!plist || !json || !length) {
        return PLIST_ERR_INVALID_ARG;
    }

    if (!PLIST_IS_DICT(plist) && !PLIST_IS_ARRAY(plist)) {
        PLIST_JSON_WRITE_ERR("plist data is not valid for JSON format\n");
        return PLIST_ERR_FORMAT;
    }

    res = node_estimate_size(plist, &size, 0, prettify);
    if (res < 0) {
        return res;
    }

    strbuf_t *outbuf = str_buf_new(size);
    if (!outbuf) {
        PLIST_JSON_WRITE_ERR("Could not allocate output buffer\n");
        return PLIST_ERR_NO_MEM;
    }

    res = node_to_json(plist, &outbuf, 0, prettify);
    if (res < 0) {
        str_buf_free(outbuf);
        *json = NULL;
        *length = 0;
        return res;
    }
    if (prettify) {
        str_buf_append(outbuf, "\n", 1);
    }

    str_buf_append(outbuf, "\0", 1);

    *json = outbuf->data;
    *length = outbuf->len - 1;

    outbuf->data = NULL;
    str_buf_free(outbuf);

    return PLIST_ERR_SUCCESS;
}

typedef struct {
    jsmntok_t* tokens;
    int count;
} jsmntok_info_t;

static int64_t parse_decimal(const char* str, const char* str_end, char** endp)
{
    uint64_t MAX = INT64_MAX;
    uint64_t x = 0;
    int is_neg = 0;
    *endp = (char*)str;

    if (str[0] == '-') {
        is_neg = 1;
        (*endp)++;
    }
    if (is_neg) {
        MAX++;
    }
    while (*endp < str_end && isdigit(**endp)) {
        if (x > PO10i_LIMIT) {
            x = MAX;
            break;
        }
        x = x * 10;
        unsigned int add = (**endp - '0');
        if (x + add > MAX) {
            x = MAX;
            break;
        }
        x += add;
        (*endp)++;
    }

    // swallow the rest of the digits in case we dropped out early
    while (*endp < str_end && isdigit(**endp)) (*endp)++;

    int64_t result = x;
    if (is_neg) {
        if (x == MAX) {
            result = INT64_MIN;
        } else {
            result = -(int64_t)x;
        }
    }
    return result;
}

static plist_t parse_primitive(const char* js, jsmntok_info_t* ti, int* index)
{
    if (ti->tokens[*index].type != JSMN_PRIMITIVE) {
        PLIST_JSON_ERR("%s: token type != JSMN_PRIMITIVE\n", __func__);
        return NULL;
    }
    plist_t val = NULL;
    const char* str_val = js + ti->tokens[*index].start;
    const char* str_end = js + ti->tokens[*index].end;
    size_t str_len = ti->tokens[*index].end - ti->tokens[*index].start;
    if (!strncmp("false", str_val, str_len)) {
        val = plist_new_bool(0);
    } else if (!strncmp("true", str_val, str_len)) {
        val = plist_new_bool(1);
    } else if (!strncmp("null", str_val, str_len)) {
        plist_data_t data = plist_new_plist_data();
        data->type = PLIST_NULL;
        val = plist_new_node(data);
    } else if (isdigit(str_val[0]) || (str_val[0] == '-' && str_val+1 < str_end && isdigit(str_val[1]))) {
        char* endp = (char*)str_val;
        int is_neg = (str_val[0] == '-');
        int64_t intpart = parse_decimal(str_val, str_end, &endp);
        if (endp >= str_end) {
            /* integer */
            if (is_neg || intpart <= INT64_MAX) {
                val = plist_new_int(intpart);
            } else {
                val = plist_new_uint((uint64_t)intpart);
            }
        } else if ((*endp == '.' && endp+1 < str_end && isdigit(*(endp+1))) || ((*endp == 'e' || *endp == 'E') && endp+1 < str_end && (isdigit(*(endp+1)) || ((*(endp+1) == '-') && endp+2 < str_end && isdigit(*(endp+2)))))) {
            /* floating point */
            double dval = (double)intpart;
            char* fendp = endp;
            int err = 0;
            do {
                if (*endp == '.') {
                    fendp++;
                    double frac = 0;
                    double p = 0.1;
                    while (fendp < str_end && isdigit(*fendp)) {
                        frac = frac + (*fendp - '0') * p;
                        p *= 0.1;
                        fendp++;
                    }
                    if (is_neg) {
                        dval -= frac;
                    } else {
                        dval += frac;
                    }
                }
                if (fendp >= str_end) {
                    break;
                }
                if (fendp+1 < str_end && (*fendp == 'e' || *fendp == 'E') && (isdigit(*(fendp+1)) || ((*(fendp+1) == '-') && fendp+2 < str_end && isdigit(*(fendp+2))))) {
                    int64_t exp = parse_decimal(fendp+1, str_end, &fendp);
                    dval = dval * pow(10, (double)exp);
                } else {
                    PLIST_JSON_ERR("%s: invalid character at offset %d when parsing floating point value\n", __func__, (int)(fendp - js));
                    err++;
                }
            } while (0);
            if (!err) {
                if (isinf(dval) || isnan(dval)) {
                   PLIST_JSON_ERR("%s: unrepresentable floating point value at offset %d when parsing numerical value\n", __func__, (int)(str_val - js));
                } else {
                    val = plist_new_real(dval);
                }
            }
        } else {
            PLIST_JSON_ERR("%s: invalid character at offset %d when parsing numerical value\n", __func__, (int)(endp - js));
        }
    } else {
        PLIST_JSON_ERR("%s: invalid primitive value '%.*s' encountered\n", __func__, (int)str_len, str_val);
    }
    (*index)++;
    return val;
}

static char* unescape_string(const char* str_val, size_t str_len, size_t *new_len)
{
    char* strval = strndup(str_val, str_len);
    size_t i = 0;
    while (i < str_len) {
        if (strval[i] == '\\' && i < str_len-1) {
            switch (strval[i+1]) {
                case '\"': case '/' : case '\\' : case 'b' :
                case 'f' : case 'r' : case 'n'  : case 't' :
                    memmove(strval+i, strval+i+1, str_len - (i+1));
                    str_len--;
                    switch (strval[i]) {
                        case 'b':
                            strval[i] = '\b';
                            break;
                        case 'f':
                            strval[i] = '\f';
                            break;
                        case 'r':
                            strval[i] = '\r';
                            break;
                        case 'n':
                            strval[i] = '\n';
                            break;
                        case 't':
                            strval[i] = '\t';
                            break;
                        default:
                            break;
                    }
                    break;
                case 'u': {
                    unsigned int val = 0;
                    if (str_len-(i+2) < 4) {
                        PLIST_JSON_ERR("%s: invalid escape sequence '%s' (too short)\n", __func__, strval+i);
                        free(strval);
                        return NULL;
                    }
                    if (!(isxdigit(strval[i+2]) && isxdigit(strval[i+3]) && isxdigit(strval[i+4]) && isxdigit(strval[i+5])) || sscanf(strval+i+2, "%04x", &val) != 1) {
                        PLIST_JSON_ERR("%s: invalid escape sequence '%.*s'\n", __func__, 6, strval+i);
                        free(strval);
                        return NULL;
                    }
                    int bytelen = 0;
                    if (val >= 0x800) {
                        /* three bytes */
                        strval[i]   = (char)(0xE0 + ((val >> 12) & 0xF));
                        strval[i+1] = (char)(0x80 + ((val >> 6) & 0x3F));
                        strval[i+2] = (char)(0x80 + (val & 0x3F));
                        bytelen = 3;
                    } else if (val >= 0x80) {
                        /* two bytes */
                        strval[i]   = (char)(0xC0 + ((val >> 6) & 0x1F));
                        strval[i+1] = (char)(0x80 + (val & 0x3F));
                        bytelen = 2;
                    } else {
                        /* one byte */
                        strval[i] = (char)(val & 0x7F);
                        bytelen = 1;
                    }
                    memmove(strval+i+bytelen, strval+i+6, str_len - (i+5));
                    str_len -= (6-bytelen);
                }   break;
                default:
                    PLIST_JSON_ERR("%s: invalid escape sequence '%.*s'\n", __func__, 2, strval+i);
                    free(strval);
                    return NULL;
            }
        }
        i++;
    }
    strval[str_len] = '\0';
    if (new_len) {
        *new_len = str_len;
    }
    return strval;
}

static plist_t parse_string(const char* js, jsmntok_info_t* ti, int* index)
{
    if (ti->tokens[*index].type != JSMN_STRING) {
        PLIST_JSON_ERR("%s: token type != JSMN_STRING\n", __func__);
        return NULL;
    }

    size_t str_len = 0; ;
    char* strval = unescape_string(js + ti->tokens[*index].start, ti->tokens[*index].end - ti->tokens[*index].start, &str_len);
    if (!strval) {
        return NULL;
    }
    plist_t node;

    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_STRING;
    data->strval = strval;
    data->length = str_len;
    node = plist_new_node(data);

    (*index)++;
    return node;
}

static plist_t parse_object(const char* js, jsmntok_info_t* ti, int* index);

static plist_t parse_array(const char* js, jsmntok_info_t* ti, int* index)
{
    if (ti->tokens[*index].type != JSMN_ARRAY) {
        PLIST_JSON_ERR("%s: token type != JSMN_ARRAY\n", __func__);
        return NULL;
    }
    plist_t arr = plist_new_array();
    int num_tokens = ti->tokens[*index].size;
    int num;
    int j = (*index)+1;
    for (num = 0; num < num_tokens; num++) {
        if (j >= ti->count) {
            PLIST_JSON_ERR("%s: token index out of valid range\n", __func__);
            plist_free(arr);
            return NULL;
        }
        plist_t val = NULL;
        switch (ti->tokens[j].type) {
            case JSMN_OBJECT:
                val = parse_object(js, ti, &j);
                break;
            case JSMN_ARRAY:
                val = parse_array(js, ti, &j);
                break;
            case JSMN_STRING:
                val = parse_string(js, ti, &j);
                break;
            case JSMN_PRIMITIVE:
                val = parse_primitive(js, ti, &j);
                break;
            default:
                break;
        }
        if (val) {
            plist_array_append_item(arr, val);
        } else {
            plist_free(arr);
            return NULL;
        }
    }
    *(index) = j;
    return arr;
}

static plist_t parse_object(const char* js, jsmntok_info_t* ti, int* index)
{
    if (ti->tokens[*index].type != JSMN_OBJECT) {
        PLIST_JSON_ERR("%s: token type != JSMN_OBJECT\n", __func__);
        return NULL;
    }
    int num_tokens = ti->tokens[*index].size;
    int num;
    int j = (*index)+1;
    if (num_tokens % 2 != 0) {
        PLIST_JSON_ERR("%s: number of children must be even\n", __func__);
        return NULL;
    }
    plist_t obj = plist_new_dict();
    for (num = 0; num < num_tokens; num++) {
        if (j+1 >= ti->count) {
            PLIST_JSON_ERR("%s: token index out of valid range\n", __func__);
            plist_free(obj);
            return NULL;
        }
        if (ti->tokens[j].type == JSMN_STRING) {
            char* key = unescape_string(js + ti->tokens[j].start, ti->tokens[j].end - ti->tokens[j].start, NULL);
            if (!key) {
                plist_free(obj);
                return NULL;
            }
            plist_t val = NULL;
            j++;
            num++;
            switch (ti->tokens[j].type) {
                case JSMN_OBJECT:
                    val = parse_object(js, ti, &j);
                    break;
                case JSMN_ARRAY:
                    val = parse_array(js, ti, &j);
                    break;
                case JSMN_STRING:
                    val = parse_string(js, ti, &j);
                    break;
                case JSMN_PRIMITIVE:
                    val = parse_primitive(js, ti, &j);
                    break;
                default:
                    break;
            }
            if (val) {
                plist_dict_set_item(obj, key, val);
            } else {
                free(key);
                plist_free(obj);
                return NULL;
            }
            free(key);
        } else {
            PLIST_JSON_ERR("%s: keys must be of type STRING\n", __func__);
            plist_free(obj);
            return NULL;
        }
    }
    (*index) = j;
    return obj;
}

PLIST_API int plist_from_json(const char *json, uint32_t length, plist_t * plist)
{
    if (!plist) {
        return PLIST_ERR_INVALID_ARG;
    }
    *plist = NULL;
    if (!json || (length == 0)) {
        return PLIST_ERR_INVALID_ARG;
    }

    jsmn_parser parser;
    jsmn_init(&parser);
    int maxtoks = 256;
    int curtoks = 0;
    int r = 0;
    jsmntok_t *tokens = NULL;

    do {
        jsmntok_t* newtokens = realloc(tokens, sizeof(jsmntok_t)*maxtoks);
        if (!newtokens) {
            PLIST_JSON_ERR("%s: Out of memory\n", __func__);
            return PLIST_ERR_NO_MEM;
        }
        memset((unsigned char*)newtokens + sizeof(jsmntok_t)*curtoks, '\0', sizeof(jsmntok_t)*(maxtoks-curtoks));
        tokens = newtokens;
        curtoks = maxtoks;

        r = jsmn_parse(&parser, json, length, tokens, maxtoks);
        if (r == JSMN_ERROR_NOMEM) {
            maxtoks+=16;
            continue;
        }
    } while (r == JSMN_ERROR_NOMEM);

    switch(r) {
        case JSMN_ERROR_NOMEM:
            PLIST_JSON_ERR("%s: Out of memory...\n", __func__);
            free(tokens);
            return PLIST_ERR_NO_MEM;
        case JSMN_ERROR_INVAL:
            PLIST_JSON_ERR("%s: Invalid character inside JSON string\n", __func__);
            free(tokens);
            return PLIST_ERR_PARSE;
        case JSMN_ERROR_PART:
            PLIST_JSON_ERR("%s: Incomplete JSON, more bytes expected\n", __func__);
            free(tokens);
            return PLIST_ERR_PARSE;
        default:
            break;
    }

    int startindex = 0;
    jsmntok_info_t ti = { tokens, parser.toknext };
    switch (tokens[startindex].type) {
        case JSMN_PRIMITIVE:
            *plist = parse_primitive(json, &ti, &startindex);
            break;
        case JSMN_STRING:
            *plist = parse_string(json, &ti, &startindex);
            break;
        case JSMN_ARRAY:
            *plist = parse_array(json, &ti, &startindex);
            break;
        case JSMN_OBJECT:
            *plist = parse_object(json, &ti, &startindex);
            break;
        default:
            break;
    }
    free(tokens);
    return PLIST_ERR_SUCCESS;
}
