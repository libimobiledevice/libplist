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
#include "bytearray.h"

bytearray_t *byte_array_new()
{
	bytearray_t *a = (bytearray_t*)malloc(sizeof(bytearray_t));
	a->data = malloc(256);
	a->len = 0;
	a->capacity = 256;
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

void byte_array_append(bytearray_t *ba, void *buf, size_t len)
{
	if (!ba || !ba->data || (len <= 0)) return;
	size_t remaining = ba->capacity-ba->len;
	if (len > remaining) {
		ba->data = realloc(ba->data, ba->capacity + (len - remaining));
		ba->capacity += (len - remaining);
	}
	memcpy(((char*)ba->data) + ba->len, buf, len);
	ba->len += len;
}
