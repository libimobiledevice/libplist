/*
 * plist.c
 * Builds plist XML structures.
 *
 * Copyright (c) 2008 Zach C. All Rights Reserved.
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
#include <assert.h>
#include "utils.h"
#include "plist.h"
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>

plist_t plist_new_node(plist_data_t data)
{
	return (plist_t) g_node_new(data);
}

plist_data_t plist_get_data(const plist_t node)
{
	if (!node)
		return NULL;
	return ((GNode *) node)->data;
}

plist_data_t plist_new_plist_data()
{
	plist_data_t data = (plist_data_t) calloc(sizeof(struct plist_data_s), 1);
	return data;
}

void plist_free_plist_data(plist_data_t data)
{
	if (data) {
		switch (data->type) {

		default:
			break;
		}
		free(data);
	}
}

plist_t plist_new_dict()
{
	plist_data_t data = plist_new_plist_data();
	data->type = PLIST_DICT;
	return plist_new_node(data);
}

plist_t plist_new_array()
{
	plist_data_t data = plist_new_plist_data();
	data->type = PLIST_ARRAY;
	return plist_new_node(data);
}

plist_t plist_add_sub_element(plist_t node, plist_type type, void *value, uint64_t length)
{
	//only structured types can have children
	plist_type node_type = plist_get_node_type(node);
	if (node_type == PLIST_DICT || node_type == PLIST_ARRAY) {
		//only structured types are allowed to have nulll value
		if (value || (!value && (type == PLIST_DICT || type == PLIST_ARRAY))) {
			//now handle value
			plist_data_t data = plist_new_plist_data();
			data->type = type;
			data->length = length;

			switch (type) {
			case PLIST_BOOLEAN:
				data->boolval = *((char *) value);
				break;
			case PLIST_UINT:
				data->intval = *((uint64_t *) value);
				break;
			case PLIST_REAL:
				data->realval = *((double *) value);
				break;
			case PLIST_KEY:
			case PLIST_STRING:
				data->strval = strdup((char *) value);
				break;
			case PLIST_UNICODE:
				data->unicodeval = wcsdup((wchar_t *) value);
				break;
			case PLIST_DATA:
				memcpy(data->buff, value, length);
				break;
			case PLIST_ARRAY:
			case PLIST_DICT:
			case PLIST_DATE:
			default:
				break;
			}

			plist_t subnode = plist_new_node(data);
			if (node)
				g_node_append(node, subnode);
			return subnode;
		} else
			return NULL;
	}
}

void plist_free(plist_t plist)
{
	g_node_destroy(plist);
}

plist_t plist_get_first_child(plist_t node)
{
	return (plist_t) g_node_first_child((GNode *) node);
}

plist_t plist_get_next_sibling(plist_t node)
{
	return (plist_t) g_node_next_sibling((GNode *) node);
}

plist_t plist_get_prev_sibling(plist_t node)
{
	return (plist_t) g_node_prev_sibling((GNode *) node);
}

static char compare_node_value(plist_type type, plist_data_t data, void *value, uint64_t length)
{
	char res = FALSE;
	switch (type) {
	case PLIST_BOOLEAN:
		res = data->boolval == *((char *) value) ? TRUE : FALSE;
		break;
	case PLIST_UINT:
		res = data->intval == *((uint64_t *) value) ? TRUE : FALSE;
		break;
	case PLIST_REAL:
		res = data->realval == *((double *) value) ? TRUE : FALSE;
		break;
	case PLIST_KEY:
	case PLIST_STRING:
		res = !strcmp(data->strval, ((char *) value));
		break;
	case PLIST_UNICODE:
		res = !wcscmp(data->unicodeval, ((wchar_t *) value));
		break;
	case PLIST_DATA:
		res = memcmp(data->buff, (char *) value, length);
		break;
	case PLIST_ARRAY:
	case PLIST_DICT:
	case PLIST_DATE:
	default:
		break;
	}
	return res;
}

plist_t plist_find_node(plist_t plist, plist_type type, void *value, uint64_t length)
{
	if (!plist)
		return NULL;

	plist_t current = NULL;
	for (current = plist_get_first_child(plist); current; current = plist_get_next_sibling(current)) {

		plist_data_t data = plist_get_data(current);

		if (data->type == type && data->length == length && compare_node_value(type, data, value, length)) {
			return current;
		}
		if (data->type == PLIST_DICT || data->type == PLIST_ARRAY) {
			plist_t sub = plist_find_node(current, type, value, length);
			if (sub)
				return sub;
		}
	}
	return NULL;
}

void plist_get_type_and_value(plist_t node, plist_type * type, void *value, uint64_t * length)
{
	if (!node)
		return;

	plist_data_t data = plist_get_data(node);

	*type = data->type;
	*length = data->length;

	switch (*type) {
	case PLIST_BOOLEAN:
		*((char *) value) = data->boolval;
		break;
	case PLIST_UINT:
		*((uint64_t *) value) = data->intval;
		break;
	case PLIST_REAL:
		*((double *) value) = data->realval;
		break;
	case PLIST_KEY:
	case PLIST_STRING:
		*((char **) value) = strdup(data->strval);
		break;
	case PLIST_UNICODE:
		*((wchar_t **) value) = wcsdup(data->unicodeval);
		break;
	case PLIST_DATA:
	case PLIST_ARRAY:
	case PLIST_DICT:
	case PLIST_DATE:
	default:
		break;
	}
}

plist_type plist_get_node_type(plist_t node)
{
	if (node) {
		plist_data_t data = plist_get_data(node);
		if (data)
			return data->type;
	}
	return PLIST_NONE;
}

uint64_t plist_get_node_uint_val(plist_t node)
{
	if (PLIST_UINT == plist_get_node_type(node))
		return plist_get_data(node)->intval;
	else
		return 0;
}

void plist_add_sub_node(plist_t node, plist_t subnode)
{
	if (node && subnode) {
		plist_type type = plist_get_node_type(node);
		if (type == PLIST_DICT || type == PLIST_ARRAY)
			g_node_append(node, subnode);
	}
}

void plist_add_sub_key_el(plist_t node, char *val)
{
	plist_add_sub_element(node, PLIST_KEY, val, strlen(val));
}

void plist_add_sub_string_el(plist_t node, char *val)
{
	plist_add_sub_element(node, PLIST_STRING, val, strlen(val));
}

void plist_add_sub_bool_el(plist_t node, uint8_t val)
{
	plist_add_sub_element(node, PLIST_BOOLEAN, &val, sizeof(uint8_t));
}

void plist_add_sub_uint_el(plist_t node, uint64_t val)
{
	plist_add_sub_element(node, PLIST_UINT, &val, sizeof(uint64_t));
}

void plist_add_sub_real_el(plist_t node, double val)
{
	plist_add_sub_element(node, PLIST_REAL, &val, sizeof(double));
}

void plist_add_sub_data_el(plist_t node, char *val, uint64_t length)
{
	plist_add_sub_element(node, PLIST_DATA, val, length);
}

void plist_get_key_val(plist_t node, char **val)
{
	plist_type type = plist_get_node_type(node);
	uint64_t length = 0;
	if (PLIST_KEY == type)
		plist_get_type_and_value(node, &type, (void *) val, &length);
	assert(length == strlen(*val));
}

void plist_get_string_val(plist_t node, char **val)
{
	plist_type type = plist_get_node_type(node);
	uint64_t length = 0;
	if (PLIST_STRING == type)
		plist_get_type_and_value(node, &type, (void *) val, &length);
	assert(length == strlen(*val));
}

void plist_get_bool_val(plist_t node, uint8_t * val)
{
	plist_type type = plist_get_node_type(node);
	uint64_t length = 0;
	if (PLIST_BOOLEAN == type)
		plist_get_type_and_value(node, &type, (void *) val, &length);
	assert(length == sizeof(uint8_t));
}

void plist_get_uint_val(plist_t node, uint64_t * val)
{
	plist_type type = plist_get_node_type(node);
	uint64_t length = 0;
	if (PLIST_UINT == type)
		plist_get_type_and_value(node, &type, (void *) val, &length);
	assert(length == sizeof(uint64_t));
}

void plist_get_real_val(plist_t node, double *val)
{
	plist_type type = plist_get_node_type(node);
	uint64_t length = 0;
	if (PLIST_REAL == type)
		plist_get_type_and_value(node, &type, (void *) val, &length);
	assert(length == sizeof(double));
}

void plist_get_data_val(plist_t node, char **val, uint64_t * length)
{
	plist_type type = plist_get_node_type(node);
	if (PLIST_UINT == type)
		plist_get_type_and_value(node, &type, (void *) val, length);
}
