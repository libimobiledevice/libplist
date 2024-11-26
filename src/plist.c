/*
 * plist.c
 * Builds plist XML structures
 *
 * Copyright (c) 2009-2023 Nikias Bassen, All Rights Reserved.
 * Copyright (c) 2010-2015 Martin Szulecki, All Rights Reserved.
 * Copyright (c) 2008 Zach C., All Rights Reserved.
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

#define _GNU_SOURCE 1
#include <string.h>
#include "plist.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <inttypes.h>

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <node.h>
#include <node_list.h>
#include <hashtable.h>
#include <ptrarray.h>

#ifdef _MSC_VER
typedef SSIZE_T ssize_t;
#endif

#ifdef DEBUG
static int plist_debug = 0;
#define PLIST_ERR(...) if (plist_debug > 0) { fprintf(stderr, "libplist ERROR: " __VA_ARGS__); }
#else
#define PLIST_ERR(...)
#endif

#ifndef bswap16
#define bswap16(x)   ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#endif

#ifndef bswap32
#define bswap32(x)   ((((x) & 0xFF000000) >> 24) \
                    | (((x) & 0x00FF0000) >>  8) \
                    | (((x) & 0x0000FF00) <<  8) \
                    | (((x) & 0x000000FF) << 24))
#endif

#ifndef bswap64
#define bswap64(x)   ((((x) & 0xFF00000000000000ull) >> 56) \
                    | (((x) & 0x00FF000000000000ull) >> 40) \
                    | (((x) & 0x0000FF0000000000ull) >> 24) \
                    | (((x) & 0x000000FF00000000ull) >>  8) \
                    | (((x) & 0x00000000FF000000ull) <<  8) \
                    | (((x) & 0x0000000000FF0000ull) << 24) \
                    | (((x) & 0x000000000000FF00ull) << 40) \
                    | (((x) & 0x00000000000000FFull) << 56))
#endif

#ifndef le16toh
#ifdef __LITTLE_ENDIAN__
#define le16toh(x) (x)
#else
#define le16toh(x) bswap16(x)
#endif
#endif

#ifndef le32toh
#ifdef __LITTLE_ENDIAN__
#define le32toh(x) (x)
#else
#define le32toh(x) bswap32(x)
#endif
#endif

#ifndef le64toh
#ifdef __LITTLE_ENDIAN__
#define le64toh(x) (x)
#else
#define le64toh(x) bswap64(x)
#endif
#endif

// Reference: https://stackoverflow.com/a/2390626/1806760
// Initializer/finalizer sample for MSVC and GCC/Clang.
// 2010-2016 Joe Lowe. Released into the public domain.

#ifdef __cplusplus
    #define INITIALIZER(f) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void)
#elif defined(_MSC_VER)
    #pragma section(".CRT$XCU",read)
    #define INITIALIZER2_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
    #ifdef _WIN64
        #define INITIALIZER(f) INITIALIZER2_(f,"")
    #else
        #define INITIALIZER(f) INITIALIZER2_(f,"_")
    #endif
#else
    #define INITIALIZER(f) \
        static void f(void) __attribute__((__constructor__)); \
        static void f(void)
#endif

extern void plist_xml_init(void);
extern void plist_xml_deinit(void);
extern void plist_bin_init(void);
extern void plist_bin_deinit(void);
extern void plist_json_init(void);
extern void plist_json_deinit(void);
extern void plist_ostep_init(void);
extern void plist_ostep_deinit(void);

static void internal_plist_deinit(void)
{
    plist_bin_deinit();
    plist_xml_deinit();
    plist_json_deinit();
    plist_ostep_deinit();
}

INITIALIZER(internal_plist_init)
{
    plist_bin_init();
    plist_xml_init();
    plist_json_init();
    plist_ostep_init();
    atexit(internal_plist_deinit);
}

#ifndef HAVE_MEMMEM
// see https://sourceware.org/legacy-ml/libc-alpha/2007-12/msg00000.html

#ifndef _LIBC
# define __builtin_expect(expr, val)   (expr)
#endif

#undef memmem

/* Return the first occurrence of NEEDLE in HAYSTACK. */
void* memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len)
{
    /* not really Rabin-Karp, just using additive hashing */
    char* haystack_ = (char*)haystack;
    char* needle_ = (char*)needle;
    int hash = 0;  /* this is the static hash value of the needle */
    int hay_hash = 0;  /* rolling hash over the haystack */
    char* last;
    size_t i;

    if (haystack_len < needle_len)
        return NULL;

    if (!needle_len)
        return haystack_;

    /* initialize hashes */
    for (i = needle_len; i; --i) {
        hash += *needle_++;
        hay_hash += *haystack_++;
    }

    /* iterate over the haystack */
    haystack_ = (char*)haystack;
    needle_ = (char*)needle;
    last = haystack_+(haystack_len - needle_len + 1);
    for (; haystack_ < last; ++haystack_) {
        if (__builtin_expect(hash == hay_hash, 0)
              && *haystack_ == *needle_ /* prevent calling memcmp, was a optimization from existing glibc */
              && !memcmp (haystack_, needle_, needle_len)) {
            return haystack_;
        }
        /* roll the hash */
        hay_hash -= *haystack_;
        hay_hash += *(haystack_+needle_len);
    }
    return NULL;
}
#endif

int plist_is_binary(const char *plist_data, uint32_t length)
{
    if (length < 8) {
        return 0;
    }

    return (memcmp(plist_data, "bplist00", 8) == 0);
}

#define SKIP_WS(blob, pos, len) \
    while (pos < len && ((blob[pos] == ' ') || (blob[pos] == '\t') || (blob[pos] == '\r') || (blob[pos] == '\n'))) pos++;
#define FIND_NEXT(blob, pos, len, chr) \
    while (pos < len && (blob[pos] != chr)) pos++;

plist_err_t plist_from_memory(const char *plist_data, uint32_t length, plist_t *plist, plist_format_t *format)
{
    plist_err_t res = PLIST_ERR_UNKNOWN;
    if (!plist) {
        return PLIST_ERR_INVALID_ARG;
    }
    *plist = NULL;
    if (!plist_data || length == 0) {
        return PLIST_ERR_INVALID_ARG;
    }
    plist_format_t fmt = PLIST_FORMAT_NONE;
    if (format) *format = PLIST_FORMAT_NONE;
    if (plist_is_binary(plist_data, length)) {
        res = plist_from_bin(plist_data, length, plist);
        fmt = PLIST_FORMAT_BINARY;
    } else {
        uint32_t pos = 0;
        int is_json = 0;
        int is_xml = 0;
        /* skip whitespace */
        SKIP_WS(plist_data, pos, length);
        if (pos >= length) {
            return PLIST_ERR_PARSE;
        }
        if (plist_data[pos] == '<' && (length-pos > 3) && !isxdigit(plist_data[pos+1]) && !isxdigit(plist_data[pos+2]) && !isxdigit(plist_data[pos+3])) {
            is_xml = 1;
        } else if (plist_data[pos] == '[') {
            /* only valid for json */
            is_json = 1;
        } else if (plist_data[pos] == '(') {
            /* only valid for openstep */
        } else if (plist_data[pos] == '{') {
            /* this could be json or openstep */
            pos++;
            SKIP_WS(plist_data, pos, length);
            if (pos >= length) {
                return PLIST_ERR_PARSE;
            }
            if (plist_data[pos] == '"') {
                /* still could be both */
                pos++;
                while (pos < length) {
                    FIND_NEXT(plist_data, pos, length, '"');
                    if (plist_data[pos-1] != '\\') {
                        break;
                    }
                    pos++;
                }
                if (pos >= length) {
                    return PLIST_ERR_PARSE;
                }
                if (plist_data[pos] == '"') {
                    pos++;
                    SKIP_WS(plist_data, pos, length);
                    if (pos >= length) {
                        return PLIST_ERR_PARSE;
                    }
                    if (plist_data[pos] == ':') {
                        /* this is definitely json */
                        is_json = 1;
                    }
                }
            }
        }
        if (is_xml) {
            res = plist_from_xml(plist_data, length, plist);
            fmt = PLIST_FORMAT_XML;
        } else if (is_json) {
            res = plist_from_json(plist_data, length, plist);
            fmt = PLIST_FORMAT_JSON;
        } else {
            res = plist_from_openstep(plist_data, length, plist);
            fmt = PLIST_FORMAT_OSTEP;
        }
    }
    if (format && res == PLIST_ERR_SUCCESS) {
        *format = fmt;
    }
    return res;
}

plist_err_t plist_read_from_file(const char *filename, plist_t *plist, plist_format_t *format)
{
    if (!filename || !plist) {
        return PLIST_ERR_INVALID_ARG;
    }
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return PLIST_ERR_IO;
    }
    struct stat fst;
    fstat(fileno(f), &fst);
    if ((uint64_t)fst.st_size > UINT32_MAX) {
        return PLIST_ERR_NO_MEM;
    }
    uint32_t total = (uint32_t)fst.st_size;
    if (total == 0) {
        return PLIST_ERR_PARSE;
    }
    char *buf = (char*)malloc(total);
    if (!buf) {
        fclose(f);
        return PLIST_ERR_NO_MEM;
    }
    uint32_t done = 0;
    while (done < total) {
        ssize_t r = fread(buf + done, 1, total - done, f);
        if (r <= 0) {
            break;
        }
        done += r;
    }
    fclose(f);
    if (done < total) {
        free(buf);
        return PLIST_ERR_IO;
    }
    plist_err_t res = plist_from_memory(buf, total, plist, format);
    free(buf);
    return res;
}

plist_t plist_new_node(plist_data_t data)
{
    return (plist_t) node_create(NULL, data);
}

plist_data_t plist_get_data(plist_t node)
{
    if (!node)
        return NULL;
    return (plist_data_t)((node_t)node)->data;
}

plist_data_t plist_new_plist_data(void)
{
    plist_data_t data = (plist_data_t) calloc(sizeof(struct plist_data_s), 1);
    return data;
}

static unsigned int dict_key_hash(const void *data)
{
    plist_data_t keydata = (plist_data_t)data;
    unsigned int hash = 5381;
    size_t i;
    char *str = keydata->strval;
    for (i = 0; i < keydata->length; str++, i++) {
        hash = ((hash << 5) + hash) + *str;
    }
    return hash;
}

static int dict_key_compare(const void* a, const void* b)
{
    plist_data_t data_a = (plist_data_t)a;
    plist_data_t data_b = (plist_data_t)b;
    if (data_a->strval == NULL || data_b->strval == NULL) {
        return FALSE;
    }
    if (data_a->length != data_b->length) {
        return FALSE;
    }
    return (strcmp(data_a->strval, data_b->strval) == 0) ? TRUE : FALSE;
}

void plist_free_data(plist_data_t data)
{
    if (data)
    {
        switch (data->type)
        {
        case PLIST_KEY:
        case PLIST_STRING:
            free(data->strval);
            break;
        case PLIST_DATA:
            free(data->buff);
            break;
        case PLIST_ARRAY:
            ptr_array_free((ptrarray_t*)data->hashtable);
            break;
        case PLIST_DICT:
            hash_table_destroy((hashtable_t*)data->hashtable);
            break;
        default:
            break;
        }
        free(data);
    }
}

static int plist_free_node(node_t node)
{
    plist_data_t data = NULL;
    int node_index = node_detach(node->parent, node);
    data = plist_get_data(node);
    plist_free_data(data);
    node->data = NULL;

    node_t ch;
    for (ch = node_first_child(node); ch; ) {
        node_t next = node_next_sibling(ch);
        plist_free_node(ch);
        ch = next;
    }

    node_destroy(node);

    return node_index;
}

plist_t plist_new_dict(void)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_DICT;
    return plist_new_node(data);
}

plist_t plist_new_array(void)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_ARRAY;
    return plist_new_node(data);
}

//These nodes should not be handled by users
static plist_t plist_new_key(const char *val)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_KEY;
    data->strval = strdup(val);
    data->length = strlen(val);
    return plist_new_node(data);
}

plist_t plist_new_string(const char *val)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_STRING;
    data->strval = strdup(val);
    data->length = strlen(val);
    return plist_new_node(data);
}

plist_t plist_new_bool(uint8_t val)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_BOOLEAN;
    data->boolval = val;
    data->length = sizeof(uint8_t);
    return plist_new_node(data);
}

plist_t plist_new_uint(uint64_t val)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_INT;
    data->intval = val;
    data->length = (val > INT_MAX) ? sizeof(uint64_t)*2 : sizeof(uint64_t);
    return plist_new_node(data);
}

plist_t plist_new_int(int64_t val)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_INT;
    data->intval = val;
    data->length = sizeof(uint64_t);
    return plist_new_node(data);
}

plist_t plist_new_uid(uint64_t val)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_UID;
    data->intval = val;
    data->length = sizeof(uint64_t);
    return plist_new_node(data);
}

plist_t plist_new_real(double val)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_REAL;
    data->realval = val;
    data->length = sizeof(double);
    return plist_new_node(data);
}

plist_t plist_new_data(const char *val, uint64_t length)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_DATA;
    data->buff = (uint8_t *) malloc(length);
    memcpy(data->buff, val, length);
    data->length = length;
    return plist_new_node(data);
}

plist_t plist_new_date(int32_t sec, int32_t usec)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_DATE;
    data->realval = (double)sec + (double)usec / 1000000;
    data->length = sizeof(double);
    return plist_new_node(data);
}

plist_t plist_new_null(void)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_NULL;
    data->intval = 0;
    data->length = 0;
    return plist_new_node(data);
}

void plist_free(plist_t plist)
{
    if (plist)
    {
        plist_free_node((node_t)plist);
    }
}

void plist_mem_free(void* ptr)
{
    if (ptr)
    {
        free(ptr);
    }
}

static plist_t plist_copy_node(node_t node)
{
    plist_type node_type = PLIST_NONE;
    plist_t newnode = NULL;
    plist_data_t data = plist_get_data(node);
    plist_data_t newdata = plist_new_plist_data();

    assert(data);				// plist should always have data
    assert(newdata);

    memcpy(newdata, data, sizeof(struct plist_data_s));

    node_type = plist_get_node_type(node);
    switch (node_type) {
        case PLIST_DATA:
            newdata->buff = (uint8_t *) malloc(data->length);
            memcpy(newdata->buff, data->buff, data->length);
            break;
        case PLIST_KEY:
        case PLIST_STRING:
            newdata->strval = strdup(data->strval);
            break;
        case PLIST_ARRAY:
            if (data->hashtable) {
                ptrarray_t* pa = ptr_array_new(((ptrarray_t*)data->hashtable)->capacity);
                assert(pa);
                newdata->hashtable = pa;
            }
            break;
        case PLIST_DICT:
            if (data->hashtable) {
                hashtable_t* ht = hash_table_new(dict_key_hash, dict_key_compare, NULL);
                assert(ht);
                newdata->hashtable = ht;
            }
            break;
        default:
            break;
    }
    newnode = plist_new_node(newdata);

    node_t ch;
    unsigned int node_index = 0;
    for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
        /* copy child node */
        plist_t newch = plist_copy_node(ch);
        /* attach to new parent node */
        node_attach((node_t)newnode, (node_t)newch);
        /* if needed, add child node to lookup table of parent node */
        switch (node_type) {
            case PLIST_ARRAY:
                if (newdata->hashtable) {
                    ptr_array_add((ptrarray_t*)newdata->hashtable, newch);
                }
                break;
            case PLIST_DICT:
                if (newdata->hashtable && (node_index % 2 != 0)) {
                    hash_table_insert((hashtable_t*)newdata->hashtable, (node_prev_sibling((node_t)newch))->data, newch);
                }
                break;
            default:
                break;
        }
        node_index++;
    }
    return newnode;
}

plist_t plist_copy(plist_t node)
{
    return node ? plist_copy_node((node_t)node) : NULL;
}

uint32_t plist_array_get_size(plist_t node)
{
    uint32_t ret = 0;
    if (node && PLIST_ARRAY == plist_get_node_type(node))
    {
        ret = node_n_children((node_t)node);
    }
    return ret;
}

plist_t plist_array_get_item(plist_t node, uint32_t n)
{
    plist_t ret = NULL;
    if (node && PLIST_ARRAY == plist_get_node_type(node) && n < INT_MAX)
    {
        ptrarray_t *pa = (ptrarray_t*)((plist_data_t)((node_t)node)->data)->hashtable;
        if (pa) {
            ret = (plist_t)ptr_array_index(pa, n);
        } else {
            ret = (plist_t)node_nth_child((node_t)node, n);
        }
    }
    return ret;
}

uint32_t plist_array_get_item_index(plist_t node)
{
    plist_t father = plist_get_parent(node);
    if (PLIST_ARRAY == plist_get_node_type(father))
    {
        return node_child_position((node_t)father, (node_t)node);
    }
    return UINT_MAX;
}

static void _plist_array_post_insert(plist_t node, plist_t item, long n)
{
    ptrarray_t *pa = (ptrarray_t*)((plist_data_t)((node_t)node)->data)->hashtable;
    if (pa) {
        /* store pointer to item in array */
        ptr_array_insert(pa, item, n);
    } else {
        if (((node_t)node)->count > 100) {
            /* make new lookup array */
            pa = ptr_array_new(128);
            plist_t current = NULL;
            for (current = (plist_t)node_first_child((node_t)node);
                 pa && current;
                 current = (plist_t)node_next_sibling((node_t)current))
            {
                ptr_array_add(pa, current);
            }
            ((plist_data_t)((node_t)node)->data)->hashtable = pa;
        }
    }
}

void plist_array_set_item(plist_t node, plist_t item, uint32_t n)
{
    if (!item) {
        return;
    }
    if (node && PLIST_ARRAY == plist_get_node_type(node) && n < INT_MAX)
    {
        plist_t old_item = plist_array_get_item(node, n);
        if (old_item)
        {
            int idx = plist_free_node((node_t)old_item);
            assert(idx >= 0);
            if (idx < 0) {
                return;
            }
            node_insert((node_t)node, idx, (node_t)item);
            ptrarray_t* pa = (ptrarray_t*)((plist_data_t)((node_t)node)->data)->hashtable;
            if (pa) {
                ptr_array_set(pa, item, idx);
            }
        }
    }
}

void plist_array_append_item(plist_t node, plist_t item)
{
    if (!item) {
        return;
    }
    if (node && PLIST_ARRAY == plist_get_node_type(node))
    {
        node_attach((node_t)node, (node_t)item);
        _plist_array_post_insert(node, item, -1);
    }
}

void plist_array_insert_item(plist_t node, plist_t item, uint32_t n)
{
    if (!item) {
        return;
    }
    if (node && PLIST_ARRAY == plist_get_node_type(node) && n < INT_MAX)
    {
        node_insert((node_t)node, n, (node_t)item);
        _plist_array_post_insert(node, item, (long)n);
    }
}

void plist_array_remove_item(plist_t node, uint32_t n)
{
    if (node && PLIST_ARRAY == plist_get_node_type(node) && n < INT_MAX)
    {
        plist_t old_item = plist_array_get_item(node, n);
        if (old_item)
        {
            ptrarray_t* pa = (ptrarray_t*)((plist_data_t)((node_t)node)->data)->hashtable;
            if (pa) {
                ptr_array_remove(pa, n);
            }
            plist_free(old_item);
        }
    }
}

void plist_array_item_remove(plist_t node)
{
    plist_t father = plist_get_parent(node);
    if (PLIST_ARRAY == plist_get_node_type(father))
    {
        int n = node_child_position((node_t)father, (node_t)node);
        if (n < 0) return;
        ptrarray_t* pa = (ptrarray_t*)((plist_data_t)((node_t)father)->data)->hashtable;
        if (pa) {
            ptr_array_remove(pa, n);
        }
        plist_free(node);
    }
}

void plist_array_new_iter(plist_t node, plist_array_iter *iter)
{
    if (iter)
    {
        *iter = malloc(sizeof(node_t));
        *((node_t*)(*iter)) = node_first_child((node_t)node);
    }
}

void plist_array_next_item(plist_t node, plist_array_iter iter, plist_t *item)
{
    node_t* iter_node = (node_t*)iter;

    if (item)
    {
        *item = NULL;
    }

    if (node && PLIST_ARRAY == plist_get_node_type(node) && *iter_node)
    {
        if (item)
        {
            *item = (plist_t)(*iter_node);
        }
        *iter_node = node_next_sibling(*iter_node);
    }
}

uint32_t plist_dict_get_size(plist_t node)
{
    uint32_t ret = 0;
    if (node && PLIST_DICT == plist_get_node_type(node))
    {
        ret = node_n_children((node_t)node) / 2;
    }
    return ret;
}

void plist_dict_new_iter(plist_t node, plist_dict_iter *iter)
{
    if (iter)
    {
        *iter = malloc(sizeof(node_t));
        *((node_t*)(*iter)) = node_first_child((node_t)node);
    }
}

void plist_dict_next_item(plist_t node, plist_dict_iter iter, char **key, plist_t *val)
{
    node_t* iter_node = (node_t*)iter;

    if (key)
    {
        *key = NULL;
    }
    if (val)
    {
        *val = NULL;
    }

    if (node && PLIST_DICT == plist_get_node_type(node) && *iter_node)
    {
        if (key)
        {
            plist_get_key_val((plist_t)(*iter_node), key);
        }
        *iter_node = node_next_sibling(*iter_node);
        if (val)
        {
            *val = (plist_t)(*iter_node);
        }
        *iter_node = node_next_sibling(*iter_node);
    }
}

void plist_dict_get_item_key(plist_t node, char **key)
{
    plist_t father = plist_get_parent(node);
    if (PLIST_DICT == plist_get_node_type(father))
    {
        plist_get_key_val( (plist_t) node_prev_sibling((node_t)node), key);
    }
}

plist_t plist_dict_item_get_key(plist_t node)
{
    plist_t ret = NULL;
    plist_t father = plist_get_parent(node);
    if (PLIST_DICT == plist_get_node_type(father))
    {
        ret = (plist_t)node_prev_sibling((node_t)node);
    }
    return ret;
}

plist_t plist_dict_get_item(plist_t node, const char* key)
{
    plist_t ret = NULL;

    if (node && PLIST_DICT == plist_get_node_type(node))
    {
        plist_data_t data = plist_get_data(node);
        hashtable_t *ht = (hashtable_t*)data->hashtable;
        if (ht) {
            struct plist_data_s sdata;
            sdata.strval = (char*)key;
            sdata.length = strlen(key);
            ret = (plist_t)hash_table_lookup(ht, &sdata);
        } else {
            plist_t current = NULL;
            for (current = (plist_t)node_first_child((node_t)node);
                current;
                current = (plist_t)node_next_sibling(node_next_sibling((node_t)current)))
            {
                data = plist_get_data(current);
                assert( PLIST_KEY == plist_get_node_type(current) );

                if (data && !strcmp(key, data->strval))
                {
                    ret = (plist_t)node_next_sibling((node_t)current);
                    break;
                }
            }
        }
    }
    return ret;
}

void plist_dict_set_item(plist_t node, const char* key, plist_t item)
{
    if (!item) {
        return;
    }
    if (node && PLIST_DICT == plist_get_node_type(node)) {
        plist_t old_item = plist_dict_get_item(node, key);
        plist_t key_node = NULL;
        if (old_item) {
            int idx = plist_free_node((node_t)old_item);
            assert(idx >= 0);
            if (idx < 0) {
                return;
            }
            node_insert((node_t)node, idx, (node_t)item);
            key_node = node_prev_sibling((node_t)item);
        } else {
            key_node = plist_new_key(key);
            node_attach((node_t)node, (node_t)key_node);
            node_attach((node_t)node, (node_t)item);
        }

        hashtable_t *ht = (hashtable_t*)((plist_data_t)((node_t)node)->data)->hashtable;
        if (ht) {
            /* store pointer to item in hash table */
            hash_table_insert(ht, (plist_data_t)((node_t)key_node)->data, item);
        } else {
            if (((node_t)node)->count > 500) {
                /* make new hash table */
                ht = hash_table_new(dict_key_hash, dict_key_compare, NULL);
                /* calculate the hashes for all entries we have so far */
                plist_t current = NULL;
                for (current = (plist_t)node_first_child((node_t)node);
                     ht && current;
                     current = (plist_t)node_next_sibling(node_next_sibling((node_t)current)))
                {
                    hash_table_insert(ht, ((node_t)current)->data, node_next_sibling((node_t)current));
                }
                ((plist_data_t)((node_t)node)->data)->hashtable = ht;
            }
        }
    }
}

void plist_dict_remove_item(plist_t node, const char* key)
{
    if (node && PLIST_DICT == plist_get_node_type(node))
    {
        plist_t old_item = plist_dict_get_item(node, key);
        if (old_item)
        {
            plist_t key_node = node_prev_sibling((node_t)old_item);
            hashtable_t* ht = (hashtable_t*)((plist_data_t)((node_t)node)->data)->hashtable;
            if (ht) {
                hash_table_remove(ht, ((node_t)key_node)->data);
            }
            plist_free(key_node);
            plist_free(old_item);
        }
    }
}

void plist_dict_merge(plist_t *target, plist_t source)
{
	if (!target || !*target || (plist_get_node_type(*target) != PLIST_DICT) || !source || (plist_get_node_type(source) != PLIST_DICT))
		return;

	char* key = NULL;
	plist_dict_iter it = NULL;
	plist_t subnode = NULL;
	plist_dict_new_iter(source, &it);
	if (!it)
		return;

	do {
		plist_dict_next_item(source, it, &key, &subnode);
		if (!key)
			break;

		plist_dict_set_item(*target, key, plist_copy(subnode));
		free(key);
		key = NULL;
	} while (1);
	free(it);
}

uint8_t plist_dict_get_bool(plist_t dict, const char *key)
{
	uint8_t bval = 0;
	uint64_t uintval = 0;
	const char *strval = NULL;
	uint64_t strsz = 0;
	plist_t node = plist_dict_get_item(dict, key);
	if (!node) {
		return 0;
	}
	switch (plist_get_node_type(node)) {
	case PLIST_BOOLEAN:
		plist_get_bool_val(node, &bval);
		break;
	case PLIST_INT:
		plist_get_uint_val(node, &uintval);
		bval = (uintval) ? 1 : 0;
		break;
	case PLIST_STRING:
		strval = plist_get_string_ptr(node, NULL);
		if (strval) {
			if (strcmp(strval, "true")) {
				bval = 1;
			} else if (strcmp(strval, "false")) {
				bval = 0;
			} else {
				PLIST_ERR("%s: invalid string '%s' for string to boolean conversion\n", __func__, strval);
			}
		}
		break;
	case PLIST_DATA:
		strval = (const char*)plist_get_data_ptr(node, &strsz);
		if (strval) {
			if (strsz == 1) {
				bval = (strval[0]) ? 1 : 0;
			} else {
				PLIST_ERR("%s: invalid size %" PRIu64 " for data to boolean conversion\n", __func__, strsz);
			}
		}
		break;
	default:
		break;
	}
	return bval;
}

int64_t plist_dict_get_int(plist_t dict, const char *key)
{
	int64_t intval = 0;
	const char *strval = NULL;
	uint64_t strsz = 0;
	plist_t node = plist_dict_get_item(dict, key);
	if (!node) {
		return intval;
	}
	switch (plist_get_node_type(node)) {
	case PLIST_INT:
		plist_get_int_val(node, &intval);
		break;
	case PLIST_STRING:
		strval = plist_get_string_ptr(node, NULL);
		if (strval) {
			intval = strtoll(strval, NULL, 0);
		}
		break;
	case PLIST_DATA:
		strval = (const char*)plist_get_data_ptr(node, &strsz);
		if (strval) {
			if (strsz == 8) {
				intval = le64toh(*(int64_t*)strval);
			} else if (strsz == 4) {
				intval = le32toh(*(int32_t*)strval);
			} else if (strsz == 2) {
				intval = le16toh(*(int16_t*)strval);
			} else if (strsz == 1) {
				intval = strval[0];
			} else {
				PLIST_ERR("%s: invalid size %" PRIu64 " for data to integer conversion\n", __func__, strsz);
			}
		}
		break;
	default:
		break;
	}
	return intval;
}


uint64_t plist_dict_get_uint(plist_t dict, const char *key)
{
	uint64_t uintval = 0;
	const char *strval = NULL;
	uint64_t strsz = 0;
	plist_t node = plist_dict_get_item(dict, key);
	if (!node) {
		return uintval;
	}
	switch (plist_get_node_type(node)) {
	case PLIST_INT:
		plist_get_uint_val(node, &uintval);
		break;
	case PLIST_STRING:
		strval = plist_get_string_ptr(node, NULL);
		if (strval) {
			uintval = strtoull(strval, NULL, 0);
		}
		break;
	case PLIST_DATA:
		strval = (const char*)plist_get_data_ptr(node, &strsz);
		if (strval) {
			if (strsz == 8) {
				uintval = le64toh(*(uint64_t*)strval);
			} else if (strsz == 4) {
				uintval = le32toh(*(uint32_t*)strval);
			} else if (strsz == 2) {
				uintval = le16toh(*(uint16_t*)strval);
			} else if (strsz == 1) {
				uintval = strval[0];
			} else {
				PLIST_ERR("%s: invalid size %" PRIu64 " for data to integer conversion\n", __func__, strsz);
			}
		}
		break;
	default:
		break;
	}
	return uintval;
}

plist_err_t plist_dict_copy_item(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key)
{
	plist_t node = plist_dict_get_item(source_dict, (alt_source_key) ? alt_source_key : key);
	if (!node) {
		return PLIST_ERR_INVALID_ARG;
	}
	plist_dict_set_item(target_dict, key, plist_copy(node));
	return PLIST_ERR_SUCCESS;
}

plist_err_t plist_dict_copy_bool(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key)
{
	if (plist_dict_get_item(source_dict, (alt_source_key) ? alt_source_key : key) == NULL) {
		return PLIST_ERR_INVALID_ARG;
	}
	uint8_t bval = plist_dict_get_bool(source_dict, (alt_source_key) ? alt_source_key : key);
	plist_dict_set_item(target_dict, key, plist_new_bool(bval));
	return PLIST_ERR_SUCCESS;
}

plist_err_t plist_dict_copy_int(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key)
{
	if (plist_dict_get_item(source_dict, (alt_source_key) ? alt_source_key : key) == NULL) {
		return PLIST_ERR_INVALID_ARG;
	}
	int64_t i64val = plist_dict_get_int(source_dict, (alt_source_key) ? alt_source_key : key);
	plist_dict_set_item(target_dict, key, plist_new_int(i64val));
	return PLIST_ERR_SUCCESS;
}

plist_err_t plist_dict_copy_uint(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key)
{
	if (plist_dict_get_item(source_dict, (alt_source_key) ? alt_source_key : key) == NULL) {
		return PLIST_ERR_INVALID_ARG;
	}
	uint64_t u64val = plist_dict_get_uint(source_dict, (alt_source_key) ? alt_source_key : key);
	plist_dict_set_item(target_dict, key, plist_new_uint(u64val));
	return PLIST_ERR_SUCCESS;
}

plist_err_t plist_dict_copy_data(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key)
{
	plist_t node = plist_dict_get_item(source_dict, (alt_source_key) ? alt_source_key : key);
	if (!PLIST_IS_DATA(node)) {
		return PLIST_ERR_INVALID_ARG;
	}
	plist_dict_set_item(target_dict, key, plist_copy(node));
	return PLIST_ERR_SUCCESS;
}

plist_err_t plist_dict_copy_string(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key)
{
	plist_t node = plist_dict_get_item(source_dict, (alt_source_key) ? alt_source_key : key);
	if (!PLIST_IS_STRING(node)) {
		return PLIST_ERR_INVALID_ARG;
	}
	plist_dict_set_item(target_dict, key, plist_copy(node));
	return PLIST_ERR_SUCCESS;
}

plist_t plist_access_pathv(plist_t plist, uint32_t length, va_list v)
{
    plist_t current = plist;
    plist_type type = PLIST_NONE;
    uint32_t i = 0;

    for (i = 0; i < length && current; i++)
    {
        type = plist_get_node_type(current);

        if (type == PLIST_ARRAY)
        {
            uint32_t n = va_arg(v, uint32_t);
            current = plist_array_get_item(current, n);
        }
        else if (type == PLIST_DICT)
        {
            const char* key = va_arg(v, const char*);
            current = plist_dict_get_item(current, key);
        }
    }
    return current;
}

plist_t plist_access_path(plist_t plist, uint32_t length, ...)
{
    plist_t ret = NULL;
    va_list v;

    va_start(v, length);
    ret = plist_access_pathv(plist, length, v);
    va_end(v);
    return ret;
}

static void plist_get_type_and_value(plist_t node, plist_type * type, void *value, uint64_t * length)
{
    plist_data_t data = NULL;

    if (!node)
        return;

    data = plist_get_data(node);

    *type = data->type;
    *length = data->length;

    switch (*type)
    {
    case PLIST_BOOLEAN:
        *((char *) value) = data->boolval;
        break;
    case PLIST_INT:
    case PLIST_UID:
        *((uint64_t *) value) = data->intval;
        break;
    case PLIST_REAL:
    case PLIST_DATE:
        *((double *) value) = data->realval;
        break;
    case PLIST_KEY:
    case PLIST_STRING:
        *((char **) value) = strdup(data->strval);
        break;
    case PLIST_DATA:
        *((uint8_t **) value) = (uint8_t *) malloc(*length * sizeof(uint8_t));
        memcpy(*((uint8_t **) value), data->buff, *length * sizeof(uint8_t));
        break;
    case PLIST_ARRAY:
    case PLIST_DICT:
    default:
        break;
    }
}

plist_t plist_get_parent(plist_t node)
{
    return node ? (plist_t) ((node_t) node)->parent : NULL;
}

plist_type plist_get_node_type(plist_t node)
{
    if (node)
    {
        plist_data_t data = plist_get_data(node);
        if (data)
            return data->type;
    }
    return PLIST_NONE;
}

void plist_get_key_val(plist_t node, char **val)
{
    if (!node || !val)
        return;
    plist_type type = plist_get_node_type(node);
    uint64_t length = 0;
    if (PLIST_KEY != type)
        return;
    plist_get_type_and_value(node, &type, (void *) val, &length);
    if (!*val)
        return;
    assert(length == strlen(*val));
}

void plist_get_string_val(plist_t node, char **val)
{
    if (!node || !val)
        return;
    plist_type type = plist_get_node_type(node);
    uint64_t length = 0;
    if (PLIST_STRING != type)
        return;
    plist_get_type_and_value(node, &type, (void *) val, &length);
    if (!*val)
        return;
    assert(length == strlen(*val));
}

const char* plist_get_string_ptr(plist_t node, uint64_t* length)
{
    if (!node)
        return NULL;
    plist_type type = plist_get_node_type(node);
    if (PLIST_STRING != type)
        return NULL;
    plist_data_t data = plist_get_data(node);
    if (length)
        *length = data->length;
    return (const char*)data->strval;
}

void plist_get_bool_val(plist_t node, uint8_t * val)
{
    if (!node || !val)
        return;
    plist_type type = plist_get_node_type(node);
    uint64_t length = 0;
    if (PLIST_BOOLEAN != type)
        return;
    plist_get_type_and_value(node, &type, (void *) val, &length);
    assert(length == sizeof(uint8_t));
}

void plist_get_uint_val(plist_t node, uint64_t * val)
{
    if (!node || !val)
        return;
    plist_type type = plist_get_node_type(node);
    uint64_t length = 0;
    if (PLIST_INT != type)
        return;
    plist_get_type_and_value(node, &type, (void *) val, &length);
    assert(length == sizeof(uint64_t) || length == 16);
}

void plist_get_int_val(plist_t node, int64_t * val)
{
    plist_get_uint_val(node, (uint64_t*)val);
}

void plist_get_uid_val(plist_t node, uint64_t * val)
{
    if (!node || !val)
        return;
    plist_type type = plist_get_node_type(node);
    uint64_t length = 0;
    if (PLIST_UID != type)
        return;
    plist_get_type_and_value(node, &type, (void *) val, &length);
    assert(length == sizeof(uint64_t));
}

void plist_get_real_val(plist_t node, double *val)
{
    if (!node || !val)
        return;
    plist_type type = plist_get_node_type(node);
    uint64_t length = 0;
    if (PLIST_REAL != type)
        return;
    plist_get_type_and_value(node, &type, (void *) val, &length);
    assert(length == sizeof(double));
}

void plist_get_data_val(plist_t node, char **val, uint64_t * length)
{
    if (!node || !val || !length)
        return;
    plist_type type = plist_get_node_type(node);
    if (PLIST_DATA != type)
        return;
    plist_get_type_and_value(node, &type, (void *) val, length);
}

const char* plist_get_data_ptr(plist_t node, uint64_t* length)
{
    if (!node || !length)
        return NULL;
    plist_type type = plist_get_node_type(node);
    if (PLIST_DATA != type)
        return NULL;
    plist_data_t data = plist_get_data(node);
    *length = data->length;
    return (const char*)data->buff;
}

void plist_get_date_val(plist_t node, int32_t * sec, int32_t * usec)
{
    if (!node)
        return;
    plist_type type = plist_get_node_type(node);
    uint64_t length = 0;
    double val = 0;
    if (PLIST_DATE != type)
        return;
    plist_get_type_and_value(node, &type, (void *) &val, &length);
    assert(length == sizeof(double));
    if (sec)
        *sec = (int32_t)val;
    if (usec)
    {
	val = fabs((val - (int64_t)val) * 1000000);
        *usec = (int32_t)val;
    }
}

int plist_data_compare(const void *a, const void *b)
{
    plist_data_t val_a = NULL;
    plist_data_t val_b = NULL;

    if (!a || !b)
        return FALSE;

    if (!((node_t) a)->data || !((node_t) b)->data)
        return FALSE;

    val_a = plist_get_data((plist_t) a);
    val_b = plist_get_data((plist_t) b);

    if (val_a->type != val_b->type)
        return FALSE;

    switch (val_a->type)
    {
    case PLIST_BOOLEAN:
    case PLIST_NULL:
    case PLIST_INT:
    case PLIST_REAL:
    case PLIST_DATE:
    case PLIST_UID:
        if (val_a->length != val_b->length)
            return FALSE;
        return val_a->intval == val_b->intval;	//it is an union so this is sufficient

    case PLIST_KEY:
    case PLIST_STRING:
        return strcmp(val_a->strval, val_b->strval) == 0;

    case PLIST_DATA:
        if (val_a->length != val_b->length)
            return FALSE;
        return memcmp(val_a->buff, val_b->buff, val_a->length) == 0;

    case PLIST_ARRAY:
    case PLIST_DICT:
        //compare pointer
        return a == b;

    default:
        break;
    }
    return FALSE;
}

char plist_compare_node_value(plist_t node_l, plist_t node_r)
{
    return plist_data_compare(node_l, node_r);
}

static void plist_set_element_val(plist_t node, plist_type type, const void *value, uint64_t length)
{
    //free previous allocated buffer
    plist_data_t data = plist_get_data(node);
    assert(data);				// a node should always have data attached

    switch (data->type)
    {
    case PLIST_KEY:
    case PLIST_STRING:
        free(data->strval);
        data->strval = NULL;
        break;
    case PLIST_DATA:
        free(data->buff);
        data->buff = NULL;
        break;
    default:
        break;
    }

    //now handle value

    data->type = type;
    data->length = length;

    switch (type)
    {
    case PLIST_BOOLEAN:
        data->boolval = *((char *) value);
        break;
    case PLIST_INT:
    case PLIST_UID:
        data->intval = *((uint64_t *) value);
        break;
    case PLIST_REAL:
    case PLIST_DATE:
        data->realval = *((double *) value);
        break;
    case PLIST_KEY:
    case PLIST_STRING:
        data->strval = strdup((char *) value);
        break;
    case PLIST_DATA:
        data->buff = (uint8_t *) malloc(length);
        memcpy(data->buff, value, length);
        break;
    case PLIST_ARRAY:
    case PLIST_DICT:
    default:
        break;
    }
}

void plist_set_key_val(plist_t node, const char *val)
{
    plist_t father = plist_get_parent(node);
    plist_t item = plist_dict_get_item(father, val);
    if (item) {
        return;
    }
    plist_set_element_val(node, PLIST_KEY, val, strlen(val));
}

void plist_set_string_val(plist_t node, const char *val)
{
    plist_set_element_val(node, PLIST_STRING, val, strlen(val));
}

void plist_set_bool_val(plist_t node, uint8_t val)
{
    plist_set_element_val(node, PLIST_BOOLEAN, &val, sizeof(uint8_t));
}

void plist_set_uint_val(plist_t node, uint64_t val)
{
    plist_set_element_val(node, PLIST_INT, &val, (val > INT64_MAX) ? sizeof(uint64_t)*2 : sizeof(uint64_t));
}

void plist_set_int_val(plist_t node, int64_t val)
{
    plist_set_element_val(node, PLIST_INT, &val, sizeof(uint64_t));
}

void plist_set_uid_val(plist_t node, uint64_t val)
{
    plist_set_element_val(node, PLIST_UID, &val, sizeof(uint64_t));
}

void plist_set_real_val(plist_t node, double val)
{
    plist_set_element_val(node, PLIST_REAL, &val, sizeof(double));
}

void plist_set_data_val(plist_t node, const char *val, uint64_t length)
{
    plist_set_element_val(node, PLIST_DATA, val, length);
}

void plist_set_date_val(plist_t node, int32_t sec, int32_t usec)
{
    double val = (double)sec + (double)usec / 1000000;
    plist_set_element_val(node, PLIST_DATE, &val, sizeof(struct timeval));
}

int plist_bool_val_is_true(plist_t boolnode)
{
    if (!PLIST_IS_BOOLEAN(boolnode)) {
        return 0;
    }
    uint8_t bv = 0;
    plist_get_bool_val(boolnode, &bv);
    return (bv == 1);
}

int plist_int_val_is_negative(plist_t intnode)
{
    if (!PLIST_IS_INT(intnode)) {
        return 0;
    }
    plist_data_t data = plist_get_data(intnode);
    if (data->length == 16) {
        return 0;
    }
    if ((int64_t)data->intval < 0) {
        return 1;
    }
    return 0;
}

int plist_int_val_compare(plist_t uintnode, int64_t cmpval)
{
    if (!PLIST_IS_INT(uintnode)) {
        return -1;
    }
    int64_t uintval = 0;
    plist_get_int_val(uintnode, &uintval);
    if (uintval == cmpval) {
        return 0;
    }

    if (uintval < cmpval) {
        return -1;
    }

    return 1;
}

int plist_uint_val_compare(plist_t uintnode, uint64_t cmpval)
{
    if (!PLIST_IS_INT(uintnode)) {
        return -1;
    }
    uint64_t uintval = 0;
    plist_get_uint_val(uintnode, &uintval);
    if (uintval == cmpval) {
        return 0;
    }

    if (uintval < cmpval) {
        return -1;
    }

    return 1;
}

int plist_uid_val_compare(plist_t uidnode, uint64_t cmpval)
{
    if (!PLIST_IS_UID(uidnode)) {
        return -1;
    }
    uint64_t uidval = 0;
    plist_get_uid_val(uidnode, &uidval);
    if (uidval == cmpval) {
        return 0;
    }

    if (uidval < cmpval) {
        return -1;
    }

    return 1;
}

int plist_real_val_compare(plist_t realnode, double cmpval)
{
    if (!PLIST_IS_REAL(realnode)) {
        return -1;
    }
    double a = 0;
    double b = cmpval;
    plist_get_real_val(realnode, &a);
    double abs_a = fabs(a);
    double abs_b = fabs(b);
    double diff = fabs(a - b);
    if (a == b) {
        return 0;
    }

    if (a == 0 || b == 0 || (abs_a + abs_b < DBL_MIN)) {
        if (diff < (DBL_EPSILON * DBL_MIN)) {
            return 0;
        }

        if (a < b) {
            return -1;
        }
    } else {
        if ((diff / fmin(abs_a + abs_b, DBL_MAX)) < DBL_EPSILON) {
            return 0;
        }

        if (a < b) {
            return -1;
        }
    }
    return 1;
}

int plist_date_val_compare(plist_t datenode, int32_t cmpsec, int32_t cmpusec)
{
    if (!PLIST_IS_DATE(datenode)) {
        return -1;
    }
    int32_t sec = 0;
    int32_t usec = 0;
    plist_get_date_val(datenode, &sec, &usec);
    uint64_t dateval = ((int64_t)sec << 32) | usec;
    uint64_t cmpval = ((int64_t)cmpsec << 32) | cmpusec;
    if (dateval == cmpval) {
        return 0;
    }

    if (dateval < cmpval) {
        return -1;
    }

    return 1;
}

int plist_string_val_compare(plist_t strnode, const char* cmpval)
{
    if (!PLIST_IS_STRING(strnode)) {
        return -1;
    }
    plist_data_t data = plist_get_data(strnode);
    return strcmp(data->strval, cmpval);
}

int plist_string_val_compare_with_size(plist_t strnode, const char* cmpval, size_t n)
{
    if (!PLIST_IS_STRING(strnode)) {
        return -1;
    }
    plist_data_t data = plist_get_data(strnode);
    return strncmp(data->strval, cmpval, n);
}

int plist_string_val_contains(plist_t strnode, const char* substr)
{
    if (!PLIST_IS_STRING(strnode)) {
        return 0;
    }
    plist_data_t data = plist_get_data(strnode);
    return (strstr(data->strval, substr) != NULL);
}

int plist_key_val_compare(plist_t keynode, const char* cmpval)
{
    if (!PLIST_IS_KEY(keynode)) {
        return -1;
    }
    plist_data_t data = plist_get_data(keynode);
    return strcmp(data->strval, cmpval);
}

int plist_key_val_compare_with_size(plist_t keynode, const char* cmpval, size_t n)
{
    if (!PLIST_IS_KEY(keynode)) {
        return -1;
    }
    plist_data_t data = plist_get_data(keynode);
    return strncmp(data->strval, cmpval, n);
}

int plist_key_val_contains(plist_t keynode, const char* substr)
{
    if (!PLIST_IS_KEY(keynode)) {
        return 0;
    }
    plist_data_t data = plist_get_data(keynode);
    return (strstr(data->strval, substr) != NULL);
}

int plist_data_val_compare(plist_t datanode, const uint8_t* cmpval, size_t n)
{
    if (!PLIST_IS_DATA(datanode)) {
        return -1;
    }
    plist_data_t data = plist_get_data(datanode);
    if (data->length < n) {
        return -1;
    }

    if (data->length > n) {
        return 1;
    }

    return memcmp(data->buff, cmpval, n);
}

int plist_data_val_compare_with_size(plist_t datanode, const uint8_t* cmpval, size_t n)
{
    if (!PLIST_IS_DATA(datanode)) {
        return -1;
    }
    plist_data_t data = plist_get_data(datanode);
    if (data->length < n) {
        return -1;
    }
    return memcmp(data->buff, cmpval, n);
}

int plist_data_val_contains(plist_t datanode, const uint8_t* cmpval, size_t n)
{
    if (!PLIST_IS_DATA(datanode)) {
        return -1;
    }
    plist_data_t data = plist_get_data(datanode);
    return (memmem(data->buff, data->length, cmpval, n) != NULL);
}

extern void plist_xml_set_debug(int debug);
extern void plist_bin_set_debug(int debug);
extern void plist_json_set_debug(int debug);
extern void plist_ostep_set_debug(int debug);

void plist_set_debug(int debug)
{
#if DEBUG
    plist_debug = debug;
#endif
    plist_xml_set_debug(debug);
    plist_bin_set_debug(debug);
    plist_json_set_debug(debug);
    plist_ostep_set_debug(debug);
}

void plist_sort(plist_t plist)
{
    if (!plist) {
        return;
    }
    if (PLIST_IS_ARRAY(plist)) {
        uint32_t n = plist_array_get_size(plist);
        uint32_t i = 0;
        for (i = 0; i < n; i++) {
            plist_sort(plist_array_get_item(plist, i));
        }
    } else if (PLIST_IS_DICT(plist)) {
        node_t node = (node_t)plist;
        node_t ch;
        for (ch = node_first_child(node); ch; ch = node_next_sibling(ch)) {
            ch = node_next_sibling(ch);
            plist_sort((plist_t)ch);
        }
        #define KEY_DATA(x) (x->data)
        #define NEXT_KEY(x) (x->next->next)
        #define KEY_STRVAL(x) ((plist_data_t)(KEY_DATA(x)))->strval
        int swapped = 0;
        do {
            swapped = 0;
            node_t lptr = NULL;
            node_t cur_key = node_first_child((node_t)plist);

            while (NEXT_KEY(cur_key) != lptr) {
                node_t next_key = NEXT_KEY(cur_key);
                if (strcmp(KEY_STRVAL(cur_key), KEY_STRVAL(next_key)) > 0) {
                    node_t cur_val = cur_key->next;
                    node_t next_val = next_key->next;
                    // we need to swap 2 consecutive nodes with the 2 after them
                    // a -> b -> [c] -> [d] -> [e] -> [f] -> g -> h
                    //              cur           next
                    // swapped:
                    // a -> b -> [e] -> [f] -> [c] -> [d] -> g -> h
                    //              next           cur
                    node_t tmp_prev = cur_key->prev;
                    node_t tmp_next = next_val->next;
                    cur_key->prev = next_val;
                    cur_val->next = tmp_next;
                    next_val->next = cur_key;
                    next_key->prev = tmp_prev;
                    if (tmp_prev) {
                        tmp_prev->next = next_key;
                    } else {
                        ((node_t)plist)->children->begin = next_key;
                    }
                    if (tmp_next) {
                        tmp_next->prev = cur_val;
                    } else {
                        ((node_t)plist)->children->end = cur_val;
                    }
                    cur_key = next_key;
                    swapped = 1;
                }
                cur_key = NEXT_KEY(cur_key);
            }
            lptr = cur_key;
        } while (swapped);
    }
}

plist_err_t plist_write_to_string(plist_t plist, char **output, uint32_t* length, plist_format_t format, plist_write_options_t options)
{
    plist_err_t err = PLIST_ERR_UNKNOWN;
    switch (format) {
        case PLIST_FORMAT_XML:
            err = plist_to_xml(plist, output, length);
            break;
        case PLIST_FORMAT_JSON:
            err = plist_to_json(plist, output, length, ((options & PLIST_OPT_COMPACT) == 0));
            break;
        case PLIST_FORMAT_OSTEP:
            err = plist_to_openstep(plist, output, length, ((options & PLIST_OPT_COMPACT) == 0));
            break;
        case PLIST_FORMAT_PRINT:
            err = plist_write_to_string_default(plist, output, length, options);
            break;
        case PLIST_FORMAT_LIMD:
            err = plist_write_to_string_limd(plist, output, length, options);
            break;
        case PLIST_FORMAT_PLUTIL:
            err = plist_write_to_string_plutil(plist, output, length, options);
            break;
        default:
            // unsupported output format
            err = PLIST_ERR_FORMAT;
            break;
    }
    return err;
}

plist_err_t plist_write_to_stream(plist_t plist, FILE *stream, plist_format_t format, plist_write_options_t options)
{
    if (!plist || !stream) {
        return PLIST_ERR_INVALID_ARG;
    }
    plist_err_t err = PLIST_ERR_UNKNOWN;
    char *output = NULL;
    uint32_t length = 0;
    switch (format) {
        case PLIST_FORMAT_BINARY:
            err = plist_to_bin(plist, &output, &length);
            break;
        case PLIST_FORMAT_XML:
            err = plist_to_xml(plist, &output, &length);
            break;
        case PLIST_FORMAT_JSON:
            err = plist_to_json(plist, &output, &length, ((options & PLIST_OPT_COMPACT) == 0));
            break;
        case PLIST_FORMAT_OSTEP:
            err = plist_to_openstep(plist, &output, &length, ((options & PLIST_OPT_COMPACT) == 0));
            break;
        case PLIST_FORMAT_PRINT:
            err = plist_write_to_stream_default(plist, stream, options);
            break;
        case PLIST_FORMAT_LIMD:
            err = plist_write_to_stream_limd(plist, stream, options);
            break;
        case PLIST_FORMAT_PLUTIL:
            err = plist_write_to_stream_plutil(plist, stream, options);
            break;
        default:
            // unsupported output format
            err = PLIST_ERR_FORMAT;
            break;
    }
    if (output && err == PLIST_ERR_SUCCESS) {
        if (fwrite(output, 1, length, stream) < length) {
            err = PLIST_ERR_IO;
        }
        free(output);
    }
    return err;
}

plist_err_t plist_write_to_file(plist_t plist, const char* filename, plist_format_t format, plist_write_options_t options)
{
    if (!plist || !filename) {
        return PLIST_ERR_INVALID_ARG;
    }
    FILE* f = fopen(filename, "wb");
    if (!f) {
        return PLIST_ERR_IO;
    }
    plist_err_t err = plist_write_to_stream(plist, f, format, options);
    fclose(f);
    return err;
}

void plist_print(plist_t plist)
{
     plist_write_to_stream(plist, stdout, PLIST_FORMAT_PRINT, PLIST_OPT_PARTIAL_DATA);
}

const char* libplist_version()
{
#ifndef PACKAGE_VERSION
#error PACKAGE_VERSION is not defined!
#endif
	return PACKAGE_VERSION;
}
