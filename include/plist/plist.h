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

//Plist creation and edition
//utilitary functions to create root nodes (supposed to be dict or array)
plist_t plist_new_dict();
plist_t plist_new_array();
//Plist edition, create a new root if node is NULL
plist_t plist_add_sub_element( plist_t node, plist_type type, void* value, uint64_t length);

//Plist edition, only work for dict and array node
void plist_add_sub_node(plist_t node, plist_t subnode);

void plist_add_sub_key_el(plist_t node, char* val);
void plist_add_sub_string_el(plist_t node, char* val);
void plist_add_sub_bool_el(plist_t node, uint8_t val);
void plist_add_sub_uint_el(plist_t node, uint64_t val);
void plist_add_sub_real_el(plist_t node, double val);
void plist_add_sub_data_el(plist_t node, char* val, uint64_t length);


//plist free
void plist_free(plist_t plist);

//plist navigation
plist_t plist_get_first_child(plist_t node);
plist_t plist_get_next_sibling(plist_t node);
plist_t plist_get_prev_sibling(plist_t node);

//utili function to find first (and only the first encountred) corresponding node
plist_t plist_find_node(plist_t plist, plist_type type, void *value, uint64_t length);
plist_t plist_find_node_by_key(plist_t plist, char *value);
plist_t plist_find_node_by_string(plist_t plist, char *value);

void plist_get_type_and_value(plist_t node, plist_type * type, void *value, uint64_t * length);

//Plist reading
plist_type plist_get_node_type(plist_t node);

void plist_get_key_val(plist_t node, char** val);
void plist_get_string_val(plist_t node, char** val);
void plist_get_bool_val(plist_t node, uint8_t* val);
void plist_get_uint_val(plist_t node, uint64_t* val);
void plist_get_real_val(plist_t node, double* val);
void plist_get_data_val(plist_t node, char** val, uint64_t* length);

//import and export functions
void plist_to_xml(plist_t plist, char **plist_xml, uint32_t * length);
void plist_to_bin(plist_t plist, char **plist_bin, uint32_t * length);

void plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist);
void plist_from_bin(const char *plist_bin, uint32_t length, plist_t * plist);

#ifdef __cplusplus
}
#endif

#endif

