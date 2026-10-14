/*
 * bytearray.c
 * simple byte array implementation
 *
 * Copyright (c) 2011 Nikias Bassen, All Rights Reserved.
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
#include <string.h>
#include <stdint.h>
#include "bytearray.h"

#define PAGE_SIZE 4096

bytearray_t *byte_array_new(size_t initial)
{
	bytearray_t *a = (bytearray_t*)malloc(sizeof(bytearray_t));
	if (!a) {
		return NULL;
	}
	if (initial > PAGE_SIZE) {
		if (initial > SIZE_MAX - (PAGE_SIZE-1)) {
			/* rounding up to the page size would overflow */
			free(a);
			return NULL;
		}
		a->capacity = (initial+(PAGE_SIZE-1)) & (~(PAGE_SIZE-1));
	} else {
		a->capacity = PAGE_SIZE;
	}
	a->data = malloc(a->capacity);
	if (!a->data) {
		free(a);
		return NULL;
	}
	a->len = 0;
	a->stream = NULL;
	return a;
}

bytearray_t *byte_array_new_for_stream(FILE *stream)
{
	bytearray_t *a = (bytearray_t*)malloc(sizeof(bytearray_t));
	if (!a) {
		return NULL;
	}
	a->capacity = (size_t)-1;
	a->data = NULL;
	a->len = 0;
	a->stream = stream;
	return a;
}

void byte_array_free(bytearray_t *ba)
{
	if (!ba) return;
	if (ba->data) {
		free(ba->data);
	}
	free(ba);
}

void byte_array_grow(bytearray_t *ba, size_t amount)
{
	if (ba->stream || !ba->data) {
		return;
	}
	size_t increase;
	void *newdata;

	if (amount > PAGE_SIZE) {
		/* round up to the page size, guarding against integer overflow */
		increase = (amount <= SIZE_MAX - (PAGE_SIZE-1)) ? ((amount+(PAGE_SIZE-1)) & (~(PAGE_SIZE-1))) : 0;
	} else {
		increase = PAGE_SIZE;
	}
	/* guard the new total capacity against integer overflow */
	newdata = (increase > 0 && increase <= SIZE_MAX - ba->capacity) ? realloc(ba->data, ba->capacity + increase) : NULL;
	if (!newdata) {
		/* out of memory or integer overflow: put the array into a failed state (data == NULL),
		 * further appends are ignored and callers must check ba->data */
		free(ba->data);
		ba->data = NULL;
		ba->len = 0;
		ba->capacity = 0;
		return;
	}
	ba->data = newdata;
	ba->capacity += increase;
}

void byte_array_append(bytearray_t *ba, void *buf, size_t len)
{
	if (!ba || (!ba->stream && !ba->data) || (len <= 0)) return;
	if (ba->stream) {
		if (fwrite(buf, 1, len, ba->stream) < len) {
#if DEBUG
			fprintf(stderr, "ERROR: Failed to write to stream.\n");
#endif
		}
	} else {
		size_t remaining;
		if (len > SIZE_MAX - ba->len) {
			/* integer overflow when computing the new total length */
			return;
		}
		remaining = ba->capacity-ba->len;
		if (len > remaining) {
			size_t needed = len - remaining;
			byte_array_grow(ba, needed);
			if (!ba->data) {
				return;
			}
		}
		memcpy(((char*)ba->data) + ba->len, buf, len);
	}
	ba->len += len;
}
