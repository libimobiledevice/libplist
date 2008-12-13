/*
 * plist.h
 * Main include of libplist
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

#ifndef LIBPLIST_H
#define LIBPLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>

typedef void* plist_t;

typedef enum {
	PLIST_BOOLEAN,
	PLIST_UINT,
	PLIST_REAL,
	PLIST_STRING,
	PLIST_UNICODE,
	PLIST_ARRAY,
	PLIST_DICT,
	PLIST_DATE,
	PLIST_DATA,
	PLIST_KEY,
	PLIST_NONE
} plist_type;

//Plist edition
void plist_new_dict(plist_t * plist);
void plist_new_array(plist_t * plist);
void plist_new_dict_in_plist(plist_t plist, plist_t * dict);
void plist_add_dict_element(plist_t dict, char *key, plist_type type, void *value, uint64_t length);
void plist_free(plist_t plist);

//plist navigation
plist_t plist_get_first_child(plist_t node);
plist_t plist_get_next_sibling(plist_t node);
plist_t plist_get_prev_sibling(plist_t node);


void plist_to_xml(plist_t plist, char **plist_xml, uint32_t * length);
void plist_to_bin(plist_t plist, char **plist_bin, uint32_t * length);

void plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist);
void plist_from_bin(const char *plist_bin, uint32_t length, plist_t * plist);

plist_t plist_find_query_node(plist_t plist, char *key, char *request);
plist_t plist_find_node(plist_t plist, plist_type type, void *value);
void plist_get_type_and_value(plist_t node, plist_type * type, void *value, uint64_t * length);




#ifdef __cplusplus
}
#endif

#endif

