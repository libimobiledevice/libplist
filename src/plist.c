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
	return (plist_t)g_node_new(data);
}

plist_data_t plist_get_data(plist_t node)
{
	if (!node)
		return NULL;
	return ((GNode*)node)->data;
}

plist_data_t plist_new_plist_data()
{
	plist_data_t data = (plist_data_t) calloc(sizeof(struct plist_data_s), 1);
	return data;
}

void plist_free_plist_data(plist_data_t data)
{
	free(data);
}

void plist_new_dict(plist_t * plist)
{
	if (*plist != NULL)
		return;
	plist_data_t data = plist_new_plist_data();
	data->type = PLIST_DICT;
	*plist = plist_new_node(data);
}

void plist_new_array(plist_t * plist)
{
	if (*plist != NULL)
		return;
	plist_data_t data = plist_new_plist_data();
	data->type = PLIST_ARRAY;
	*plist = plist_new_node(data);
}

void plist_new_dict_in_plist(plist_t plist, plist_t * dict)
{
	if (!plist || *dict)
		return;

	plist_data_t data = plist_new_plist_data();
	data->type = PLIST_DICT;
	*dict = plist_new_node(data);
	g_node_append(plist, *dict);
}


/** Adds a new key pair to a dict.
 *
 * @param dict The dict node in the plist.
 * @param key the key name of the key pair.
 * @param type The the type of the value in the key pair.
 * @param value a pointer to the actual buffer containing the value. WARNING : the buffer is supposed to match the type of the value
 *
 */
void plist_add_dict_element(plist_t dict, char *key, plist_type type, void *value, uint64_t length)
{
	if (!dict || !key || !value)
		return;

	plist_data_t data = plist_new_plist_data();
	data->type = PLIST_KEY;
	data->strval = strdup(key);
	plist_t keynode = plist_new_node(data);
	g_node_append(dict, keynode);

	//now handle value
	plist_data_t val = plist_new_plist_data();
	val->type = type;
	val->length = length;

	switch (type) {
	case PLIST_BOOLEAN:
		val->boolval = *((char *) value);
		break;
	case PLIST_UINT:
		val->intval = *((uint64_t *) value);
		break;
	case PLIST_REAL:
		val->realval = *((double *) value);
		break;
	case PLIST_STRING:
		val->strval = strdup((char *) value);
		break;
	case PLIST_UNICODE:
		val->unicodeval = wcsdup((wchar_t *) value);
		break;
	case PLIST_DATA:
		memcpy(val->buff, value, length);
		break;
	case PLIST_ARRAY:
	case PLIST_DICT:
	case PLIST_DATE:
	default:
		break;
	}
	plist_t valnode = plist_new_node(val);
	g_node_append(dict, valnode);
}

void plist_free(plist_t plist)
{
	g_node_destroy(plist);
}

plist_t plist_get_first_child(plist_t node)
{
	return (plist_t)g_node_first_child( (GNode*)node );
}

plist_t plist_get_next_sibling(plist_t node)
{
	return (plist_t)g_node_next_sibling( (GNode*)node );
}

plist_t plist_get_prev_sibling(plist_t node)
{
	return (plist_t)g_node_prev_sibling( (GNode*)node );
}

plist_t plist_find_query_node(plist_t plist, char *key, char *request)
{
	if (!plist)
		return NULL;

	plist_t current = NULL;
	plist_t next = NULL;
	for (current = plist_get_first_child(plist); current; current = next) {

		next = plist_get_next_sibling(current);
		plist_data_t data = plist_get_data(current);

		if (data->type == PLIST_KEY && !strcmp(data->strval, key) && next) {

			data = plist_get_data(next);
			if (data->type == PLIST_STRING && !strcmp(data->strval, request))
				return next;
		}
		if (data->type == PLIST_DICT || data->type == PLIST_ARRAY) {
			plist_t sub = plist_find_query_node(current, key, request);
			if (sub)
				return sub;
		}
	}
	return NULL;
}

char compare_node_value(plist_type type, plist_data_t data, void *value)
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
		res = !strcmp(data->buff, ((char *) value));
		break;
	case PLIST_ARRAY:
	case PLIST_DICT:
	case PLIST_DATE:
	default:
		break;
	}
	return res;
}

plist_t plist_find_node(plist_t plist, plist_type type, void *value)
{
	if (!plist)
		return NULL;

	plist_t current = NULL;
	for (current = plist_get_first_child(plist); current; current = plist_get_next_sibling(current)) {

		plist_data_t data = plist_get_data(current);

		if (data->type == type && compare_node_value(type, data, value)) {
			return current;
		}
		if (data->type == PLIST_DICT || data->type == PLIST_ARRAY) {
			plist_t sub = plist_find_node(current, type, value);
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
	case PLIST_STRING:
		*((char **) value) = strdup(data->strval);
		break;
	case PLIST_UNICODE:
		*((wchar_t **) value) = wcsdup(data->unicodeval);
		break;
	case PLIST_KEY:
		*((char **) value) = strdup(data->strval);
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
