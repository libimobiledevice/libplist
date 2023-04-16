/*
 * out-limd.c
 * libplist *output-only* format introduced by libimobiledevice/ideviceinfo
 *  - NOT for machine parsing
 *
 * Copyright (c) 2022-2023 Nikias Bassen All Rights Reserved.
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
#include "time64.h"
#include "base64.h"

#define MAC_EPOCH 978307200

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

static int node_to_string(node_t node, bytearray_t **outbuf, uint32_t depth, uint32_t indent)
{
    plist_data_t node_data = NULL;

    char *val = NULL;
    size_t val_len = 0;
    char buf[16];

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

        len = node_data->length;
        for (j = 0; j < len; j++) {
            unsigned char ch = (unsigned char)node_data->strval[j];
            if (ch < 0x20) {
                str_buf_append(*outbuf, node_data->strval + start, cur - start);
                str_buf_append(*outbuf, charmap[ch], (charmap[ch][1] == 'u') ? 6 : 2);
                start = cur+1;
            }
            cur++;
        }
        str_buf_append(*outbuf, node_data->strval + start, cur - start);
        } break;

    case PLIST_ARRAY: {
        node_t ch;
        uint32_t cnt = 0;
        for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
            if (cnt > 0 || (cnt == 0 && node->parent != NULL)) {
                str_buf_append(*outbuf, "\n", 1);
                for (i = 0; i < depth+indent; i++) {
                    str_buf_append(*outbuf, " ", 1);
                }
            }
            size_t sl = sprintf(buf, "%u: ", cnt);
            str_buf_append(*outbuf, buf, sl);
            int res = node_to_string(ch, outbuf, depth+1, indent);
            if (res < 0) {
                return res;
            }
            cnt++;
        }
        } break;
    case PLIST_DICT: {
        node_t ch;
        uint32_t cnt = 0;
        for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
            if (cnt > 0 && cnt % 2 == 0) {
                str_buf_append(*outbuf, "\n", 1);
                for (i = 0; i < depth+indent; i++) {
                    str_buf_append(*outbuf, " ", 1);
                }
            }
            int res = node_to_string(ch, outbuf, depth+1, indent);
            if (res < 0) {
                return res;
            }
            if (cnt % 2 == 0) {
                plist_t valnode = (plist_t)node_next_sibling(ch);
                if (PLIST_IS_ARRAY(valnode)) {
                    size_t sl = sprintf(buf, "[%u]:", plist_array_get_size(valnode));
                    str_buf_append(*outbuf, buf, sl);
                } else {
                    str_buf_append(*outbuf, ": ", 2);
                }
            }
            cnt++;
        }
        } break;
    case PLIST_DATA:
        {
            val = (char*)malloc(4096);
            size_t done = 0;
            while (done < node_data->length) {
                size_t amount = node_data->length - done;
                if (amount > 3072) {
                    amount = 3072;
                }
                size_t bsize = base64encode(val, node_data->buff + done, amount);
                str_buf_append(*outbuf, val, bsize);
                done += amount;
            }
        }
        break;
    case PLIST_DATE:
        {
            Time64_T timev = (Time64_T)node_data->realval + MAC_EPOCH;
            struct TM _btime;
            struct TM *btime = gmtime64_r(&timev, &_btime);
            if (btime) {
                val = (char*)calloc(1, 24);
                struct tm _tmcopy;
                copy_TM64_to_tm(btime, &_tmcopy);
                val_len = strftime(val, 24, "%Y-%m-%dT%H:%M:%SZ", &_tmcopy);
                if (val_len > 0) {
                    str_buf_append(*outbuf, val, val_len);
                }
                free(val);
                val = NULL;
            }
        }
        break;
    case PLIST_UID:
        {
            str_buf_append(*outbuf, "CF$UID:", 7);
            val = (char*)malloc(64);
            if (node_data->length == 16) {
                val_len = snprintf(val, 64, "%"PRIu64, node_data->intval);
            } else {
                val_len = snprintf(val, 64, "%"PRIi64, node_data->intval);
            }
            str_buf_append(*outbuf, val, val_len);
            free(val);
        }
        break;
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

static int node_estimate_size(node_t node, uint64_t *size, uint32_t depth, uint32_t indent)
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
            int res = node_estimate_size(ch, size, depth + 1, indent);
            if (res < 0) {
                return res;
            }
        }
        switch (data->type) {
        case PLIST_DICT:
            *size += n_children-1; // number of ':' and ' '
            *size += n_children; // number of '\n' and extra space
            *size += (uint64_t)n_children * (depth+indent+1); // indent for every 2nd child
            *size += indent+1; // additional '\n'
            break;
        case PLIST_ARRAY:
            *size += n_children-1; // number of ','
            *size += n_children; // number of '\n'
            *size += (uint64_t)n_children * ((depth+indent+1)<<1); // indent for every child
            *size += indent+1; // additional '\n'
            break;
        default:
            break;
	}
    } else {
        switch (data->type) {
        case PLIST_STRING:
        case PLIST_KEY:
            *size += data->length;
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
            *size += 3;
            break;
        case PLIST_DATA:
            *size += (data->length / 3) * 4 + 4;
            break;
        case PLIST_DATE:
            *size += 23;
            break;
        case PLIST_UID:
            *size += 7; // "CF$UID:"
            *size += num_digits_u(data->intval);
            break;
        default:
#ifdef DEBUG
            fprintf(stderr, "%s: invalid node type encountered\n", __func__);
#endif
            return PLIST_ERR_UNKNOWN;
        }
    }
    if (depth == 0) {
        *size += 1; // final newline
    }
    return PLIST_ERR_SUCCESS;
}

static plist_err_t _plist_write_to_strbuf(plist_t plist, strbuf_t *outbuf, plist_write_options_t options)
{
    uint8_t indent = 0;
    if (options & PLIST_OPT_INDENT) {
        indent = (options >> 24) & 0xFF;
    }
    uint8_t i;
    for (i = 0; i < indent; i++) {
        str_buf_append(outbuf, " ", 1);
    }
    int res = node_to_string(plist, &outbuf, 0, indent);
    if (res < 0) {
        return res;
    }
    if (!(options & PLIST_OPT_NO_NEWLINE)) {
        str_buf_append(outbuf, "\n", 1);
    }
    return res;
}

plist_err_t plist_write_to_string_limd(plist_t plist, char **output, uint32_t* length, plist_write_options_t options)
{
    uint64_t size = 0;
    int res;

    if (!plist || !output || !length) {
        return PLIST_ERR_INVALID_ARG;
    }

    uint8_t indent = 0;
    if (options & PLIST_OPT_INDENT) {
        indent = (options >> 24) & 0xFF;
    }

    res = node_estimate_size(plist, &size, 0, indent);
    if (res < 0) {
        return res;
    }

    strbuf_t *outbuf = str_buf_new(size);
    if (!outbuf) {
#if DEBUG
        fprintf(stderr, "%s: Could not allocate output buffer\n", __func__);
#endif
        return PLIST_ERR_NO_MEM;
    }

    res = _plist_write_to_strbuf(plist, outbuf, options);
    if (res < 0) {
        str_buf_free(outbuf);
        *output = NULL;
        *length = 0;
        return res;
    }
    str_buf_append(outbuf, "\0", 1);

    *output = outbuf->data;
    *length = outbuf->len - 1;

    outbuf->data = NULL;
    str_buf_free(outbuf);

    return PLIST_ERR_SUCCESS;
}

plist_err_t plist_write_to_stream_limd(plist_t plist, FILE *stream, plist_write_options_t options)
{
    if (!plist || !stream) {
        return PLIST_ERR_INVALID_ARG;
    }
    strbuf_t *outbuf = str_buf_new_for_stream(stream);
    if (!outbuf) {
#if DEBUG
        fprintf(stderr, "%s: Could not allocate output buffer\n", __func__);
#endif
        return PLIST_ERR_NO_MEM;
    }

    int res = _plist_write_to_strbuf(plist, outbuf, options);
    if (res < 0) {
        str_buf_free(outbuf);
        return res;
    }

    str_buf_free(outbuf);

    return PLIST_ERR_SUCCESS;
}
