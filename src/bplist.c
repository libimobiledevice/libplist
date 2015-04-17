/*
 * plist.c
 * Binary plist implementation
 *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libxml/encoding.h>
#include <ctype.h>

#include <plist/plist.h>
#include "plist.h"
#include "hashtable.h"
#include "bytearray.h"
#include "ptrarray.h"

#include <node.h>
#include <node_iterator.h>

/* Magic marker and size. */
#define BPLIST_MAGIC		((uint8_t*)"bplist")
#define BPLIST_MAGIC_SIZE	6

#define BPLIST_VERSION		((uint8_t*)"00")
#define BPLIST_VERSION_SIZE	2


#define BPLIST_TRL_SIZE 	26
#define BPLIST_TRL_OFFSIZE_IDX 	0
#define BPLIST_TRL_PARMSIZE_IDX 1
#define BPLIST_TRL_NUMOBJ_IDX 	2
#define BPLIST_TRL_ROOTOBJ_IDX 	10
#define BPLIST_TRL_OFFTAB_IDX 	18

enum
{
    BPLIST_NULL = 0x00,
    BPLIST_FALSE = 0x08,
    BPLIST_TRUE = 0x09,
    BPLIST_FILL = 0x0F,			/* will be used for length grabbing */
    BPLIST_UINT = 0x10,
    BPLIST_REAL = 0x20,
    BPLIST_DATE = 0x30,
    BPLIST_DATA = 0x40,
    BPLIST_STRING = 0x50,
    BPLIST_UNICODE = 0x60,
    BPLIST_UNK_0x70 = 0x70,
    BPLIST_UID = 0x80,
    BPLIST_ARRAY = 0xA0,
    BPLIST_SET = 0xC0,
    BPLIST_DICT = 0xD0,
    BPLIST_MASK = 0xF0
};

static void float_byte_convert(uint8_t * address, size_t size)
{
#if (defined(__LITTLE_ENDIAN__) \
		&& !defined(__FLOAT_WORD_ORDER__)) \
	|| (defined(__FLOAT_WORD_ORDER__) \
		&& __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    uint8_t i = 0, j = 0;
    uint8_t tmp = 0;

    for (i = 0; i < (size / 2); i++)
    {
        tmp = address[i];
        j = ((size - 1) + 0) - i;
        address[i] = address[j];
        address[j] = tmp;
    }
#endif
}

union plist_uint_ptr
{
    void *src;
    uint8_t *u8ptr;
    uint16_t *u16ptr;
    uint32_t *u32ptr;
    uint64_t *u64ptr;
};

#define get_unaligned(ptr)			  \
  ({                                              \
    struct __attribute__((packed)) {		  \
      typeof(*(ptr)) __v;			  \
    } *__p = (void *) (ptr);			  \
    __p->__v;					  \
  })


static void byte_convert(uint8_t * address, size_t size)
{
#ifdef __LITTLE_ENDIAN__
    uint8_t i = 0, j = 0;
    uint8_t tmp = 0;

    for (i = 0; i < (size / 2); i++)
    {
        tmp = address[i];
        j = ((size - 1) + 0) - i;
        address[i] = address[j];
        address[j] = tmp;
    }
#endif
}

static uint32_t uint24_from_be(union plist_uint_ptr buf)
{
    union plist_uint_ptr tmp;
    uint32_t ret = 0;

    tmp.src = &ret;

    memcpy(tmp.u8ptr + 1, buf.u8ptr, 3 * sizeof(char));

    byte_convert(tmp.u8ptr, sizeof(uint32_t));
    return ret;
}

#ifndef be16toh
#ifdef __BIG_ENDIAN__
#define be16toh(x) (x)
#else
#define be16toh(x) ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#endif
#endif

#ifndef be32toh
#ifdef __BIG_ENDIAN__
#define be32toh(x) (x)
#else
#define be32toh(x) ((((x) & 0xFF000000) >> 24) \
                    | (((x) & 0x00FF0000) >> 8) \
                    | (((x) & 0x0000FF00) << 8) \
                    | (((x) & 0x000000FF) << 24))
#endif
#endif

#ifndef be64toh
#ifdef __BIG_ENDIAN__
#define be64toh(x) (x)
#else
#define be64toh(x) ((((x) & 0xFF00000000000000ull) >> 56) \
                    | (((x) & 0x00FF000000000000ull) >> 40) \
                    | (((x) & 0x0000FF0000000000ull) >> 24) \
                    | (((x) & 0x000000FF00000000ull) >> 8) \
                    | (((x) & 0x00000000FF000000ull) << 8) \
                    | (((x) & 0x0000000000FF0000ull) << 24) \
                    | (((x) & 0x000000000000FF00ull) << 40) \
                    | (((x) & 0x00000000000000FFull) << 56)) 
#endif
#endif

#ifndef _MSC_VER
#define UINT_TO_HOST(x, n) \
	({ \
		union plist_uint_ptr __up; \
		__up.src = x; \
		(n == 8 ? be64toh( get_unaligned(__up.u64ptr) ) : \
		(n == 4 ? be32toh( get_unaligned(__up.u32ptr) ) : \
		(n == 3 ? uint24_from_be( __up ) : \
		(n == 2 ? be16toh( get_unaligned(__up.u16ptr) ) : \
		*__up.u8ptr )))); \
	})
#else
uint64_t get_unaligned_64(uint64_t *ptr)
{
#pragma pack(push, 1)
    struct packed {
        uint64_t __v;
    } *__p = (packed *)(ptr);
#pragma pack(pop)	
    return __p->__v;
}

uint32_t get_unaligned_32(uint32_t *ptr)
{
#pragma pack(push, 1)	
    struct packed {
        uint32_t __v;
    } *__p = (packed *)(ptr);
#pragma pack(pop)
    return __p->__v;
}

uint16_t get_unaligned_16(uint16_t *ptr)
{
#pragma pack(push, 1)	
    struct packed {
        uint16_t __v;
    } *__p = (packed *)(ptr);
#pragma pack(pop)	
    return __p->__v;
}

uint64_t UINT_TO_HOST(void *x, uint8_t n)
{
    union plist_uint_ptr __up;
    __up.src = x;
    return (n == 8 ? be64toh(get_unaligned_64(__up.u64ptr)) :
        (n == 4 ? be32toh(get_unaligned_32(__up.u32ptr)) :
        (n == 3 ? uint24_from_be(__up) :
        (n == 2 ? be16toh(get_unaligned_16(__up.u16ptr)) :
        *__up.u8ptr))));
}
#endif

#ifndef _MSC_VER
#define be64dec(x) \
	({ \
		union plist_uint_ptr __up; \
		__up.src = x; \
		be64toh( get_unaligned(__up.u64ptr) ); \
	})
#else
uint64_t be64dec(char *x)
{
    union plist_uint_ptr __up;
    __up.src = x;
    return be64toh(get_unaligned_64(__up.u64ptr));
}
#endif

#define get_needed_bytes(x) \
		( ((uint64_t)x) < (1ULL << 8) ? 1 : \
		( ((uint64_t)x) < (1ULL << 16) ? 2 : \
		( ((uint64_t)x) < (1ULL << 24) ? 3 : \
		( ((uint64_t)x) < (1ULL << 32) ? 4 : 8))))

#define get_real_bytes(x) (x == (float) x ? 4 : 8)

#define NODE_IS_ROOT(x) (((node_t*)x)->isRoot)

static plist_t parse_uint_node(char *bnode, uint8_t size, char **next_object)
{
    plist_data_t data = plist_new_plist_data();

    size = 1 << size;			// make length less misleading
    switch (size)
    {
    case sizeof(uint8_t):
    case sizeof(uint16_t):
    case sizeof(uint32_t):
    case sizeof(uint64_t):
        memcpy(&data->intval, bnode, size);
        data->intval = UINT_TO_HOST(&data->intval, size);
        data->length = sizeof(uint64_t);
        break;
    case 16:
        memcpy(&data->intval, bnode+8, sizeof(uint64_t));
        data->intval = UINT_TO_HOST(&data->intval, sizeof(uint64_t));
        data->length = size;
        break;
    default:
        free(data);
        return NULL;
    };

    *next_object = bnode + size;
    data->type = PLIST_UINT;

    return node_create(NULL, data);
}

static plist_t parse_real_node(char *bnode, uint8_t size)
{
    plist_data_t data = plist_new_plist_data();
    float floatval = 0.0;
    uint8_t* buf;

    size = 1 << size;			// make length less misleading
    buf = (uint8_t*)malloc (size);
    memcpy (buf, bnode, size);
    switch (size)
    {
    case sizeof(float):
        float_byte_convert(buf, size);
        floatval = *(float *) buf;
        data->realval = floatval;
        break;
    case sizeof(double):
        float_byte_convert(buf, size);
        data->realval = *(double *) buf;
        break;
    default:
        free(buf);
        free(data);
        return NULL;
    }
    free (buf);
    data->type = PLIST_REAL;
    data->length = sizeof(double);

    return node_create(NULL, data);
}

static plist_t parse_date_node(char *bnode, uint8_t size)
{
    plist_t node = parse_real_node(bnode, size);
    plist_data_t data = plist_get_data(node);

    double time_real = data->realval;
    data->timeval.tv_sec = (long) time_real;
    data->timeval.tv_usec = (time_real - (long) time_real) * 1000000;
    data->type = PLIST_DATE;
    data->length = sizeof(struct timeval);

    return node;
}

static plist_t parse_string_node(char *bnode, uint64_t size)
{
    plist_data_t data = plist_new_plist_data();

    data->type = PLIST_STRING;
    data->strval = (char *) malloc(sizeof(char) * (size + 1));
    memcpy(data->strval, bnode, size);
    data->strval[size] = '\0';
    data->length = strlen(data->strval);

    return node_create(NULL, data);
}

static char *plist_utf16_to_utf8(uint16_t *unistr, long len, long *items_read, long *items_written)
{
	if (!unistr || (len <= 0)) return NULL;
	char *outbuf = (char*)malloc(4*(len+1));
	int p = 0;
	int i = 0;

	uint16_t wc;
	uint32_t w;
	int read_lead_surrogate = 0; 

	while (i < len) {
		wc = unistr[i++];
		if (wc >= 0xD800 && wc <= 0xDBFF) {
			if (!read_lead_surrogate) {
				read_lead_surrogate = 1;
				w = 0x010000 + ((wc & 0x3FF) << 10);
			} else {
				// This is invalid, the next 16 bit char should be a trail surrogate. 
				// Handling error by skipping.
				read_lead_surrogate = 0;
			}
		} else if (wc >= 0xDC00 && wc <= 0xDFFF) {
			if (read_lead_surrogate) {
				read_lead_surrogate = 0;
				w = w | (wc & 0x3FF);
				outbuf[p++] = (char)(0xF0 + ((w >> 18) & 0x3));
				outbuf[p++] = (char)(0x80 + ((w >> 12) & 0x3F));
				outbuf[p++] = (char)(0x80 + ((w >> 6) & 0x3F));
				outbuf[p++] = (char)(0x80 + (w & 0x3F));
			} else {
				// This is invalid.  A trail surrogate should always follow a lead surrogate.
				// Handling error by skipping
			}
		} else if (wc >= 0x800) {
			outbuf[p++] = (char)(0xE0 + ((wc >> 12) & 0xF));
			outbuf[p++] = (char)(0x80 + ((wc >> 6) & 0x3F));
			outbuf[p++] = (char)(0x80 + (wc & 0x3F));
		} else if (wc >= 0x80) {
			outbuf[p++] = (char)(0xC0 + ((wc >> 6) & 0x1F));
			outbuf[p++] = (char)(0x80 + (wc & 0x3F));
		} else {
			outbuf[p++] = (char)(wc & 0x7F);
		}
	}
	if (items_read) {
		*items_read = i;
	}
	if (items_written) {
		*items_written = p;
	}
	outbuf[p] = 0;

	return outbuf;
}

static plist_t parse_unicode_node(char *bnode, uint64_t size)
{
    plist_data_t data = plist_new_plist_data();
    uint64_t i = 0;
    uint16_t *unicodestr = NULL;
    char *tmpstr = NULL;
    long items_read = 0;
    long items_written = 0;

    data->type = PLIST_STRING;
    unicodestr = (uint16_t*) malloc(sizeof(uint16_t) * size);
    memcpy(unicodestr, bnode, sizeof(uint16_t) * size);
    for (i = 0; i < size; i++)
        byte_convert((uint8_t *) (unicodestr + i), sizeof(uint16_t));

    tmpstr = plist_utf16_to_utf8(unicodestr, size, &items_read, &items_written);
    free(unicodestr);

    data->type = PLIST_STRING;
    data->strval = (char *) malloc(sizeof(char) * (items_written + 1));
    memcpy(data->strval, tmpstr, items_written);
    data->strval[items_written] = '\0';
    data->length = strlen(data->strval);
    free(tmpstr);
    return node_create(NULL, data);
}

static plist_t parse_data_node(char *bnode, uint64_t size)
{
    plist_data_t data = plist_new_plist_data();

    data->type = PLIST_DATA;
    data->length = size;
    data->buff = (uint8_t *) malloc(sizeof(uint8_t) * size);
    memcpy(data->buff, bnode, sizeof(uint8_t) * size);

    return node_create(NULL, data);
}

static plist_t parse_dict_node(char *bnode, uint64_t size, uint32_t ref_size)
{
    plist_data_t data = plist_new_plist_data();

    data->type = PLIST_DICT;
    data->length = size;
    data->buff = (uint8_t *) malloc(sizeof(uint8_t) * size * ref_size * 2);
    memcpy(data->buff, bnode, sizeof(uint8_t) * size * ref_size * 2);

    return node_create(NULL, data);
}

static plist_t parse_array_node(char *bnode, uint64_t size, uint32_t ref_size)
{
    plist_data_t data = plist_new_plist_data();

    data->type = PLIST_ARRAY;
    data->length = size;
    data->buff = (uint8_t *) malloc(sizeof(uint8_t) * size * ref_size);
    memcpy(data->buff, bnode, sizeof(uint8_t) * size * ref_size);

    return node_create(NULL, data);
}

static plist_t parse_uid_node(char *bnode, uint8_t size, char **next_object)
{
    plist_data_t data = plist_new_plist_data();

    size = 1 << size;			// make length less misleading
    switch (size)
    {
    case sizeof(uint8_t):
    case sizeof(uint16_t):
    case sizeof(uint32_t):
    case sizeof(uint64_t):
        memcpy(&data->intval, bnode, size);
        data->intval = UINT_TO_HOST(&data->intval, size);
        break;
    default:
        free(data);
        return NULL;
    };

    *next_object = bnode + size;
    data->type = PLIST_UID;
    data->length = sizeof(uint64_t);

    return node_create(NULL, data);
}


static plist_t parse_bin_node(char *object, uint8_t dict_size, char **next_object)
{
    uint16_t type = 0;
    uint64_t size = 0;

    if (!object)
        return NULL;

    type = (*object) & 0xF0;
    size = (*object) & 0x0F;
    object++;

    switch (type)
    {

    case BPLIST_NULL:
        switch (size)
        {

        case BPLIST_TRUE:
        {
            plist_data_t data = plist_new_plist_data();
            data->type = PLIST_BOOLEAN;
            data->boolval = TRUE;
            data->length = 1;
            return node_create(NULL, data);
        }

        case BPLIST_FALSE:
        {
            plist_data_t data = plist_new_plist_data();
            data->type = PLIST_BOOLEAN;
            data->boolval = FALSE;
            data->length = 1;
            return node_create(NULL, data);
        }

        case BPLIST_NULL:
        default:
            return NULL;
        }

    case BPLIST_UINT:
        return parse_uint_node(object, size, next_object);

    case BPLIST_REAL:
        return parse_real_node(object, size);

    case BPLIST_DATE:
        if (3 != size)
            return NULL;
        else
            return parse_date_node(object, size);

    case BPLIST_DATA:
        if (0x0F == size)
        {
            plist_t size_node = parse_bin_node(object, dict_size, &object);
            if (plist_get_node_type(size_node) != PLIST_UINT)
                return NULL;
            plist_get_uint_val(size_node, &size);
            plist_free(size_node);
        }
        return parse_data_node(object, size);

    case BPLIST_STRING:
        if (0x0F == size)
        {
            plist_t size_node = parse_bin_node(object, dict_size, &object);
            if (plist_get_node_type(size_node) != PLIST_UINT)
                return NULL;
            plist_get_uint_val(size_node, &size);
            plist_free(size_node);
        }
        return parse_string_node(object, size);

    case BPLIST_UNICODE:
        if (0x0F == size)
        {
            plist_t size_node = parse_bin_node(object, dict_size, &object);
            if (plist_get_node_type(size_node) != PLIST_UINT)
                return NULL;
            plist_get_uint_val(size_node, &size);
            plist_free(size_node);
        }
        return parse_unicode_node(object, size);

    case BPLIST_UNK_0x70:
    case BPLIST_ARRAY:
        if (0x0F == size)
        {
            plist_t size_node = parse_bin_node(object, dict_size, &object);
            if (plist_get_node_type(size_node) != PLIST_UINT)
                return NULL;
            plist_get_uint_val(size_node, &size);
            plist_free(size_node);
        }
        return parse_array_node(object, size, dict_size);

    case BPLIST_UID:
        return parse_uid_node(object, size, next_object);

    case BPLIST_SET:
    case BPLIST_DICT:
        if (0x0F == size)
        {
            plist_t size_node = parse_bin_node(object, dict_size, &object);
            if (plist_get_node_type(size_node) != PLIST_UINT)
                return NULL;
            plist_get_uint_val(size_node, &size);
            plist_free(size_node);
        }
        return parse_dict_node(object, size, dict_size);
    default:
        return NULL;
    }
    return NULL;
}

static void* copy_plist_data(const void* src)
{
    plist_data_t srcdata = (plist_data_t) src;
    plist_data_t dstdata = plist_new_plist_data();

    dstdata->type = srcdata->type;
    dstdata->length = srcdata->length;
    switch (dstdata->type)
    {
    case PLIST_BOOLEAN:
        dstdata->boolval = srcdata->boolval;
        break;
    case PLIST_UINT:
        dstdata->intval = srcdata->intval;
        break;
    case PLIST_DATE:
        dstdata->timeval.tv_sec = srcdata->timeval.tv_sec;
        dstdata->timeval.tv_usec = srcdata->timeval.tv_usec;
        break;
    case PLIST_REAL:
        dstdata->realval = srcdata->realval;
        break;
    case PLIST_KEY:
    case PLIST_STRING:
        dstdata->strval = strdup(srcdata->strval);
        break;
    case PLIST_DATA:
    case PLIST_ARRAY:
        dstdata->buff = (uint8_t*) malloc(sizeof(uint8_t) * srcdata->length);
        memcpy(dstdata->buff, srcdata->buff, sizeof(uint8_t) * srcdata->length);
        break;
    case PLIST_DICT:
        dstdata->buff = (uint8_t*) malloc(sizeof(uint8_t) * srcdata->length * 2);
        memcpy(dstdata->buff, srcdata->buff, sizeof(uint8_t) * srcdata->length * 2);
        break;
    case PLIST_UID:
        dstdata->intval = srcdata->intval;
        break;
    default:
        break;
    }

    return dstdata;
}

PLIST_API void plist_from_bin(const char *plist_bin, uint32_t length, plist_t * plist)
{
    char *trailer = NULL;

    uint8_t offset_size = 0;
    uint8_t dict_param_size = 0;
    uint64_t num_objects = 0;
    uint64_t root_object = 0;
    uint64_t offset_table_index = 0;

    plist_t *nodeslist = NULL;
    uint64_t i = 0;
    uint64_t current_offset = 0;
    char *offset_table = NULL;
    uint32_t j = 0, str_i = 0, str_j = 0;
    uint32_t index1 = 0, index2 = 0;


    //first check we have enough data
    if (!(length >= BPLIST_MAGIC_SIZE + BPLIST_VERSION_SIZE + BPLIST_TRL_SIZE))
        return;
    //check that plist_bin in actually a plist
    if (memcmp(plist_bin, BPLIST_MAGIC, BPLIST_MAGIC_SIZE) != 0)
        return;
    //check for known version
    if (memcmp(plist_bin + BPLIST_MAGIC_SIZE, BPLIST_VERSION, BPLIST_VERSION_SIZE) != 0)
        return;

    //now parse trailer
    trailer = (char *) (plist_bin + (length - BPLIST_TRL_SIZE));

    offset_size = trailer[BPLIST_TRL_OFFSIZE_IDX];
    dict_param_size = trailer[BPLIST_TRL_PARMSIZE_IDX];
    num_objects = be64dec(trailer + BPLIST_TRL_NUMOBJ_IDX);
    root_object = be64dec(trailer + BPLIST_TRL_ROOTOBJ_IDX);
    offset_table_index = be64dec(trailer + BPLIST_TRL_OFFTAB_IDX);

    if (num_objects == 0)
        return;

    //allocate serialized array of nodes
    nodeslist = (plist_t *) malloc(sizeof(plist_t) * num_objects);

    if (!nodeslist)
        return;

    //parse serialized nodes
    offset_table = (char *) (plist_bin + offset_table_index);
    for (i = 0; i < num_objects; i++)
    {
        char *obj = NULL;
        current_offset = UINT_TO_HOST(offset_table + i * offset_size, offset_size);

        obj = (char *) (plist_bin + current_offset);
        nodeslist[i] = parse_bin_node(obj, dict_param_size, &obj);
    }

    //setup children for structured types
    for (i = 0; i < num_objects; i++)
    {

        plist_data_t data = plist_get_data(nodeslist[i]);
	if (!data) {
		break;
	}

        switch (data->type)
        {
        case PLIST_DICT:
            for (j = 0; j < data->length; j++)
            {
                node_t* n = NULL;
                str_i = j * dict_param_size;
                str_j = (j + data->length) * dict_param_size;

                index1 = UINT_TO_HOST(data->buff + str_i, dict_param_size);
                index2 = UINT_TO_HOST(data->buff + str_j, dict_param_size);

                // process key node
                if (index1 < num_objects)
                {
                    // is node already attached?
                    if (NODE_IS_ROOT(nodeslist[index1])) {
                        // use original
                        n = (node_t*)nodeslist[index1];
                    } else {
                        // use a copy, because this node is already attached elsewhere
                        n = node_copy_deep((node_t*)nodeslist[index1], copy_plist_data);
                    }

                    // enforce key type
                    plist_get_data(n)->type = PLIST_KEY;

                    // attach key node
                    node_attach((node_t*)nodeslist[i], n);
                }

                // process value node
                if (index2 < num_objects)
                {
                    // is node already attached?
                    if (NODE_IS_ROOT(nodeslist[index2])) {
                        // use original
                        n = (node_t*)nodeslist[index2];
                    } else {
                        // use a copy, because this node is already attached elsewhere
                        n = node_copy_deep((node_t*)nodeslist[index2], copy_plist_data);

                        // ensure key type is never used for values, especially if we copy a key node
                        if (plist_get_data(n)->type == PLIST_KEY) {
                            plist_get_data(n)->type = PLIST_STRING;
                        }
                    }

                    // attach value node
                    node_attach((node_t*)nodeslist[i], n);
                }
            }

            free(data->buff);
            break;

        case PLIST_ARRAY:
            for (j = 0; j < data->length; j++)
            {
                str_j = j * dict_param_size;
                index1 = UINT_TO_HOST(data->buff + str_j, dict_param_size);

                if (index1 < num_objects)
                {
                    if (NODE_IS_ROOT(nodeslist[index1]))
                        node_attach((node_t*)nodeslist[i], (node_t*)nodeslist[index1]);
                    else
                        node_attach((node_t*)nodeslist[i], node_copy_deep((node_t*)nodeslist[index1], copy_plist_data));
                }
            }
            free(data->buff);
            break;
        default:
            break;
        }
    }

    *plist = nodeslist[root_object];

    // free unreferenced nodes that would otherwise leak memory
    for (i = 0; i < num_objects; i++) {
        if (i == root_object) continue;
        node_t* node = (node_t*)nodeslist[i];
        if (node && NODE_IS_ROOT(node)) {
            plist_free(node);
        }
    }
    free(nodeslist);
}

static unsigned int plist_data_hash(const void* key)
{
    plist_data_t data = plist_get_data((plist_t) key);

    unsigned int hash = data->type;
    unsigned int i = 0;

    char *buff = NULL;
    unsigned int size = 0;

    switch (data->type)
    {
    case PLIST_BOOLEAN:
    case PLIST_UINT:
    case PLIST_REAL:
    case PLIST_UID:
        buff = (char *) &data->intval;	//works also for real as we use an union
        size = 8;
        break;
    case PLIST_KEY:
    case PLIST_STRING:
        buff = data->strval;
        size = strlen(buff);
        break;
    case PLIST_DATA:
    case PLIST_ARRAY:
    case PLIST_DICT:
        //for these types only hash pointer
        buff = (char *) &key;
        size = sizeof(const void*);
        break;
    case PLIST_DATE:
        buff = (char *) &(data->timeval);
        size = data->length;
        break;
    default:
        break;
    }

    //now perform hash
    for (i = 0; i < size; buff++, i++)
        hash = hash << 7 ^ (*buff);

    return hash;
}

struct serialize_s
{
    ptrarray_t* objects;
    hashtable_t* ref_table;
};

static void serialize_plist(node_t* node, void* data)
{
    uint64_t *index_val = NULL;
    struct serialize_s *ser = (struct serialize_s *) data;
    uint64_t current_index = ser->objects->len;

    //first check that node is not yet in objects
    void* val = hash_table_lookup(ser->ref_table, node);
    if (val)
    {
        //data is already in table
        return;
    }
    //insert new ref
    index_val = (uint64_t *) malloc(sizeof(uint64_t));
    *index_val = current_index;
    hash_table_insert(ser->ref_table, node, index_val);

    //now append current node to object array
    ptr_array_add(ser->objects, node);

    //now recurse on children
    node_iterator_t *ni = node_iterator_create(node->children);
    node_t *ch;
    while ((ch = node_iterator_next(ni))) {
        serialize_plist(ch, data);
    }
    node_iterator_destroy(ni);

    return;
}

#define Log2(x) (x == 8 ? 3 : (x == 4 ? 2 : (x == 2 ? 1 : 0)))

static void write_int(bytearray_t * bplist, uint64_t val)
{
    uint64_t size = get_needed_bytes(val);
    uint8_t *buff = NULL;
    //do not write 3bytes int node
    if (size == 3)
        size++;

#ifdef __BIG_ENDIAN__
    val = val << ((sizeof(uint64_t) - size) * 8);
#endif

    buff = (uint8_t *) malloc(sizeof(uint8_t) + size);
    buff[0] = BPLIST_UINT | Log2(size);
    memcpy(buff + 1, &val, size);
    byte_convert(buff + 1, size);
    byte_array_append(bplist, buff, sizeof(uint8_t) + size);
    free(buff);
}

static void write_uint(bytearray_t * bplist, uint64_t val)
{
    uint64_t size = 16;
    uint8_t *buff = NULL;

    buff = (uint8_t *) malloc(sizeof(uint8_t) + size);
    buff[0] = BPLIST_UINT | 4;
    memset(buff + 1, '\0', 8);
    memcpy(buff + 9, &val, 8);
    byte_convert(buff + 9, 8);
    byte_array_append(bplist, buff, sizeof(uint8_t) + size);
    free(buff);
}

static void write_real(bytearray_t * bplist, double val)
{
    uint64_t size = get_real_bytes(val);	//cheat to know used space
    uint8_t *buff = (uint8_t *) malloc(sizeof(uint8_t) + size);
    buff[0] = BPLIST_REAL | Log2(size);
    if (size == sizeof(double))
    {
        memcpy(buff + 1, &val, size);
    }
    else if (size == sizeof(float))
    {
        float tmpval = (float) val;
        memcpy(buff + 1, &tmpval, size);
    }
    float_byte_convert(buff + 1, size);
    byte_array_append(bplist, buff, sizeof(uint8_t) + size);
    free(buff);
}

static void write_date(bytearray_t * bplist, double val)
{
    uint64_t size = 8;			//dates always use 8 bytes
    uint8_t *buff = (uint8_t *) malloc(sizeof(uint8_t) + size);
    buff[0] = BPLIST_DATE | Log2(size);
    memcpy(buff + 1, &val, size);
    float_byte_convert(buff + 1, size);
    byte_array_append(bplist, buff, sizeof(uint8_t) + size);
    free(buff);
}

static void write_raw_data(bytearray_t * bplist, uint8_t mark, uint8_t * val, uint64_t size)
{
    uint8_t *buff = NULL;
    uint8_t marker = mark | (size < 15 ? size : 0xf);
    byte_array_append(bplist, &marker, sizeof(uint8_t));
    if (size >= 15)
    {
        bytearray_t *int_buff = byte_array_new();
        write_int(int_buff, size);
        byte_array_append(bplist, int_buff->data, int_buff->len);
        byte_array_free(int_buff);
    }
    //stupid unicode buffer length
    if (BPLIST_UNICODE==mark) size *= 2;
    buff = (uint8_t *) malloc(size);
    memcpy(buff, val, size);
    byte_array_append(bplist, buff, size);
    free(buff);
}

static void write_data(bytearray_t * bplist, uint8_t * val, uint64_t size)
{
    write_raw_data(bplist, BPLIST_DATA, val, size);
}

static void write_string(bytearray_t * bplist, char *val)
{
    uint64_t size = strlen(val);
    write_raw_data(bplist, BPLIST_STRING, (uint8_t *) val, size);
}

static void write_unicode(bytearray_t * bplist, uint16_t * val, uint64_t size)
{
    uint64_t i = 0;
    uint64_t size2 = size * sizeof(uint16_t);
    uint8_t *buff = (uint8_t *) malloc(size2);
    memcpy(buff, val, size2);
    for (i = 0; i < size; i++)
        byte_convert(buff + i * sizeof(uint16_t), sizeof(uint16_t));
    write_raw_data(bplist, BPLIST_UNICODE, buff, size);
    free(buff);
}

static void write_array(bytearray_t * bplist, node_t* node, hashtable_t* ref_table, uint8_t dict_param_size)
{
    uint64_t idx = 0;
    uint8_t *buff = NULL;

    node_t* cur = NULL;
    uint64_t i = 0;

    uint64_t size = node_n_children(node);
    uint8_t marker = BPLIST_ARRAY | (size < 15 ? size : 0xf);
    byte_array_append(bplist, &marker, sizeof(uint8_t));
    if (size >= 15)
    {
        bytearray_t *int_buff = byte_array_new();
        write_int(int_buff, size);
        byte_array_append(bplist, int_buff->data, int_buff->len);
        byte_array_free(int_buff);
    }

    buff = (uint8_t *) malloc(size * dict_param_size);

    for (i = 0, cur = node_first_child(node); cur && i < size; cur = node_next_sibling(cur), i++)
    {
        idx = *(uint64_t *) (hash_table_lookup(ref_table, cur));
#ifdef __BIG_ENDIAN__
	idx = idx << ((sizeof(uint64_t) - dict_param_size) * 8);
#endif
        memcpy(buff + i * dict_param_size, &idx, dict_param_size);
        byte_convert(buff + i * dict_param_size, dict_param_size);
    }

    //now append to bplist
    byte_array_append(bplist, buff, size * dict_param_size);
    free(buff);

}

static void write_dict(bytearray_t * bplist, node_t* node, hashtable_t* ref_table, uint8_t dict_param_size)
{
    uint64_t idx1 = 0;
    uint64_t idx2 = 0;
    uint8_t *buff = NULL;

    node_t* cur = NULL;
    uint64_t i = 0;

    uint64_t size = node_n_children(node) / 2;
    uint8_t marker = BPLIST_DICT | (size < 15 ? size : 0xf);
    byte_array_append(bplist, &marker, sizeof(uint8_t));
    if (size >= 15)
    {
        bytearray_t *int_buff = byte_array_new();
        write_int(int_buff, size);
        byte_array_append(bplist, int_buff->data, int_buff->len);
        byte_array_free(int_buff);
    }

    buff = (uint8_t *) malloc(size * 2 * dict_param_size);
    for (i = 0, cur = node_first_child(node); cur && i < size; cur = node_next_sibling(node_next_sibling(cur)), i++)
    {
        idx1 = *(uint64_t *) (hash_table_lookup(ref_table, cur));
#ifdef __BIG_ENDIAN__
	idx1 = idx1 << ((sizeof(uint64_t) - dict_param_size) * 8);
#endif
        memcpy(buff + i * dict_param_size, &idx1, dict_param_size);
        byte_convert(buff + i * dict_param_size, dict_param_size);

        idx2 = *(uint64_t *) (hash_table_lookup(ref_table, cur->next));
#ifdef __BIG_ENDIAN__
	idx2 = idx2 << ((sizeof(uint64_t) - dict_param_size) * 8);
#endif
        memcpy(buff + (i + size) * dict_param_size, &idx2, dict_param_size);
        byte_convert(buff + (i + size) * dict_param_size, dict_param_size);
    }

    //now append to bplist
    byte_array_append(bplist, buff, size * 2 * dict_param_size);
    free(buff);

}

static void write_uid(bytearray_t * bplist, uint64_t val)
{
    uint64_t size = get_needed_bytes(val);
    uint8_t *buff = NULL;
    //do not write 3bytes int node
    if (size == 3)
        size++;

#ifdef __BIG_ENDIAN__
    val = val << ((sizeof(uint64_t) - size) * 8);
#endif

    buff = (uint8_t *) malloc(sizeof(uint8_t) + size);
    buff[0] = BPLIST_UID | Log2(size);
    memcpy(buff + 1, &val, size);
    byte_convert(buff + 1, size);
    byte_array_append(bplist, buff, sizeof(uint8_t) + size);
    free(buff);
}

static int is_ascii_string(char* s, int len)
{
  int ret = 1, i = 0;
  for(i = 0; i < len; i++)
  {
      if ( !isascii( s[i] ) )
      {
          ret = 0;
          break;
      }
  }
  return ret;
}

static uint16_t *plist_utf8_to_utf16(char *unistr, long size, long *items_read, long *items_written)
{
	uint16_t *outbuf = (uint16_t*)malloc(((size*2)+1)*sizeof(uint16_t));
	int p = 0;
	int i = 0;

	unsigned char c0;
	unsigned char c1;
	unsigned char c2;
	unsigned char c3;

	uint32_t w;

	while (i < size) {
		c0 = unistr[i];
		c1 = (i < size-1) ? unistr[i+1] : 0;
		c2 = (i < size-2) ? unistr[i+2] : 0;
		c3 = (i < size-3) ? unistr[i+3] : 0;
		if ((c0 >= 0xF0) && (i < size-3) && (c1 >= 0x80) && (c2 >= 0x80) && (c3 >= 0x80)) {
			// 4 byte sequence.  Need to generate UTF-16 surrogate pair
			w = ((((c0 & 7) << 18) + ((c1 & 0x3F) << 12) + ((c2 & 0x3F) << 6) + (c3 & 0x3F)) & 0x0FFFFF) - 0x010000;
			outbuf[p++] = 0xD800 + (w >> 10);
			outbuf[p++] = 0xDC00 + (w & 0x3FF);
			i+=4;
		} else if ((c0 >= 0xE0) && (i < size-2) && (c1 >= 0x80) && (c2 >= 0x80)) {
			// 3 byte sequence
			outbuf[p++] = ((c2 & 0x3F) + ((c1 & 3) << 6)) + (((c1 >> 2) & 15) << 8) + ((c0 & 15) << 12);
			i+=3;
		} else if ((c0 >= 0xC0) && (i < size-1) && (c1 >= 0x80)) {
			// 2 byte sequence
			outbuf[p++] = ((c1 & 0x3F) + ((c0 & 3) << 6)) + (((c0 >> 2) & 7) << 8);
			i+=2;
		} else if (c0 < 0x80) {
			// 1 byte sequence
			outbuf[p++] = c0;
			i+=1;
		} else {
			// invalid character
			fprintf(stderr, "invalid utf8 sequence in string at index %d\n", i);
			break;
		}
	}
	if (items_read) {
		*items_read = i;
	}
	if (items_written) {
		*items_written = p;
	}
	outbuf[p] = 0;

	return outbuf;

}

PLIST_API void plist_to_bin(plist_t plist, char **plist_bin, uint32_t * length)
{
    ptrarray_t* objects = NULL;
    hashtable_t* ref_table = NULL;
    struct serialize_s ser_s;
    uint8_t offset_size = 0;
    uint8_t dict_param_size = 0;
    uint64_t num_objects = 0;
    uint64_t root_object = 0;
    uint64_t offset_table_index = 0;
    bytearray_t *bplist_buff = NULL;
    uint64_t i = 0;
    uint8_t *buff = NULL;
    uint64_t *offsets = NULL;
    uint8_t pad[6] = { 0, 0, 0, 0, 0, 0 };
    uint8_t trailer[BPLIST_TRL_SIZE];
    //for string
    long len = 0;
    long items_read = 0;
    long items_written = 0;
    uint16_t *unicodestr = NULL;
    uint64_t objects_len = 0;
    uint64_t buff_len = 0;

    //check for valid input
    if (!plist || !plist_bin || *plist_bin || !length)
        return;

    //list of objects
    objects = ptr_array_new(256);
    //hashtable to write only once same nodes
    ref_table = hash_table_new(plist_data_hash, plist_data_compare);

    //serialize plist
    ser_s.objects = objects;
    ser_s.ref_table = ref_table;
    serialize_plist((node_t*)plist, &ser_s);

    //now stream to output buffer
    offset_size = 0;			//unknown yet
    objects_len = objects->len;
    dict_param_size = get_needed_bytes(objects_len);
    num_objects = objects->len;
    root_object = 0;			//root is first in list
    offset_table_index = 0;		//unknown yet

    //setup a dynamic bytes array to store bplist in
    bplist_buff = byte_array_new();

    //set magic number and version
    byte_array_append(bplist_buff, BPLIST_MAGIC, BPLIST_MAGIC_SIZE);
    byte_array_append(bplist_buff, BPLIST_VERSION, BPLIST_VERSION_SIZE);

    //write objects and table
    offsets = (uint64_t *) malloc(num_objects * sizeof(uint64_t));
    for (i = 0; i < num_objects; i++)
    {

        plist_data_t data = plist_get_data(ptr_array_index(objects, i));
        offsets[i] = bplist_buff->len;

        switch (data->type)
        {
        case PLIST_BOOLEAN:
            buff = (uint8_t *) malloc(sizeof(uint8_t));
            buff[0] = data->boolval ? BPLIST_TRUE : BPLIST_FALSE;
            byte_array_append(bplist_buff, buff, sizeof(uint8_t));
            free(buff);
            break;

        case PLIST_UINT:
            if (data->length == 16) {
                write_uint(bplist_buff, data->intval);
            } else {
                write_int(bplist_buff, data->intval);
            }
            break;

        case PLIST_REAL:
            write_real(bplist_buff, data->realval);
            break;

        case PLIST_KEY:
        case PLIST_STRING:
            len = strlen(data->strval);
            if ( is_ascii_string(data->strval, len) )
            {
                write_string(bplist_buff, data->strval);
            }
            else
            {
                unicodestr = plist_utf8_to_utf16(data->strval, len, &items_read, &items_written);
                write_unicode(bplist_buff, unicodestr, items_written);
                free(unicodestr);
            }
            break;
        case PLIST_DATA:
            write_data(bplist_buff, data->buff, data->length);
        case PLIST_ARRAY:
            write_array(bplist_buff, (node_t*)ptr_array_index(objects, i), ref_table, dict_param_size);
            break;
        case PLIST_DICT:
            write_dict(bplist_buff, (node_t*)ptr_array_index(objects, i), ref_table, dict_param_size);
            break;
        case PLIST_DATE:
            write_date(bplist_buff, data->timeval.tv_sec + (double) data->timeval.tv_usec / 1000000);
            break;
        case PLIST_UID:
            write_uid(bplist_buff, data->intval);
            break;
        default:
            break;
        }
    }

    //free intermediate objects
    ptr_array_free(objects);
    hash_table_destroy(ref_table);

    //write offsets
    buff_len = bplist_buff->len;
    offset_size = get_needed_bytes(buff_len);
    offset_table_index = bplist_buff->len;
    for (i = 0; i < num_objects; i++)
    {
        uint8_t *offsetbuff = (uint8_t *) malloc(offset_size);

#ifdef __BIG_ENDIAN__
	offsets[i] = offsets[i] << ((sizeof(uint64_t) - offset_size) * 8);
#endif

        memcpy(offsetbuff, &offsets[i], offset_size);
        byte_convert(offsetbuff, offset_size);
        byte_array_append(bplist_buff, offsetbuff, offset_size);
        free(offsetbuff);
    }

    //experimental pad to reflect apple's files
    byte_array_append(bplist_buff, pad, 6);

    //setup trailer
    num_objects = be64toh(num_objects);
    root_object = be64toh(root_object);
    offset_table_index = be64toh(offset_table_index);

    memcpy(trailer + BPLIST_TRL_OFFSIZE_IDX, &offset_size, sizeof(uint8_t));
    memcpy(trailer + BPLIST_TRL_PARMSIZE_IDX, &dict_param_size, sizeof(uint8_t));
    memcpy(trailer + BPLIST_TRL_NUMOBJ_IDX, &num_objects, sizeof(uint64_t));
    memcpy(trailer + BPLIST_TRL_ROOTOBJ_IDX, &root_object, sizeof(uint64_t));
    memcpy(trailer + BPLIST_TRL_OFFTAB_IDX, &offset_table_index, sizeof(uint64_t));

    byte_array_append(bplist_buff, trailer, BPLIST_TRL_SIZE);

    //duplicate buffer
    *plist_bin = (char *) malloc(bplist_buff->len);
    memcpy(*plist_bin, bplist_buff->data, bplist_buff->len);
    *length = bplist_buff->len;

    byte_array_free(bplist_buff);
    free(offsets);
}
