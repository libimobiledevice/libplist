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
#include "plist.h"
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

plist_data_t plist_new_plist_data(void)
{
	plist_data_t data = (plist_data_t) calloc(sizeof(struct plist_data_s), 1);
	return data;
}

static void plist_free_node(GNode * node, gpointer none)
{
	g_node_unlink(node);
	plist_data_t data = plist_get_data(node);
	if (data) {
		switch (data->type) {
		case PLIST_KEY:
		case PLIST_STRING:
			free(data->strval);
			break;
		case PLIST_DATA:
			free(data->buff);
			break;
		default:
			break;
		}
		free(data);
	}
	node->data = NULL;
	g_node_children_foreach(node, G_TRAVERSE_ALL, plist_free_node, NULL);
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

static plist_t plist_add_sub_element(plist_t node, plist_type type, const void *value, uint64_t length)
{
	//only structured types can have children
	plist_type node_type = plist_get_node_type(node);
	if (node_type == PLIST_DICT || node_type == PLIST_ARRAY) {
		//only structured types are allowed to have nulll value
		if (value || (!value && (type == PLIST_DICT || type == PLIST_ARRAY))) {

			plist_t subnode = NULL;

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
			case PLIST_DATA:
				data->buff = (uint8_t *) malloc(length);
				memcpy(data->buff, value, length);
				break;
			case PLIST_DATE:
				data->timeval.tv_sec = ((GTimeVal *) value)->tv_sec;
				data->timeval.tv_usec = ((GTimeVal *) value)->tv_usec;
				break;
			case PLIST_ARRAY:
			case PLIST_DICT:
			default:
				break;
			}

			subnode = plist_new_node(data);
			if (node)
				g_node_append(node, subnode);
			return subnode;
		} else
			return NULL;
	}
	return NULL;
}

void plist_free(plist_t plist)
{
	plist_free_node(plist, NULL);
	g_node_destroy(plist);
}

static void plist_copy_node(GNode * node, gpointer parent_node_ptr)
{
	plist_t newnode = NULL;
	plist_data_t data = plist_get_data(node);
	plist_data_t newdata = plist_new_plist_data();

	assert(data);				// plist should always have data

	memcpy(newdata, data, sizeof(struct plist_data_s));

	plist_type node_type = plist_get_node_type(node);
	if (node_type == PLIST_DATA || node_type == PLIST_STRING || node_type == PLIST_KEY) {
		switch (node_type) {
		case PLIST_DATA:
			newdata->buff = (uint8_t *) malloc(data->length);
			memcpy(newdata->buff, data->buff, data->length);
		case PLIST_KEY:
		case PLIST_STRING:
			newdata->strval = strdup((char *) data->strval);
		default:
			break;
		}
	}
	newnode = plist_new_node(newdata);

	if (*(plist_t*)parent_node_ptr) {
		g_node_append(*(plist_t*)parent_node_ptr, newnode);
	}
	else {
		*(plist_t*)parent_node_ptr = newnode;
	}

	g_node_children_foreach(node, G_TRAVERSE_ALL, plist_copy_node, &newnode);
}

plist_t plist_copy(plist_t node)
{
	plist_t copied = NULL;
	plist_copy_node(node, &copied);
	return copied;
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

plist_t plist_get_parent(plist_t node)
{
	return node ? (plist_t) ((GNode *) node)->parent : NULL;
}

plist_t plist_get_array_nth_el(plist_t node, uint32_t n)
{
	plist_t ret = NULL;
	if (node && PLIST_ARRAY == plist_get_node_type(node)) {
		uint32_t i = 0;
		plist_t temp = plist_get_first_child(node);

		while (i <= n && temp) {
			if (i == n)
				ret = temp;
			temp = plist_get_next_sibling(temp);
			i++;
		}
	}
	return ret;
}

plist_t plist_get_dict_el_from_key(plist_t node, const char *key)
{
	plist_t ret = NULL;
	if (node && PLIST_DICT == plist_get_node_type(node)) {

		plist_t key_node = plist_find_node_by_key(node, key);
		ret = plist_get_next_sibling(key_node);
	}
	return ret;
}

static char compare_node_value(plist_type type, plist_data_t data, const void *value, uint64_t length)
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
	case PLIST_DATA:
		res = !memcmp(data->buff, (char *) value, length);
		break;
	case PLIST_DATE:
		res = !memcmp(&(data->timeval), value, sizeof(GTimeVal));
		break;
	case PLIST_ARRAY:
	case PLIST_DICT:
	default:
		break;
	}
	return res;
}

static plist_t plist_find_node(plist_t plist, plist_type type, const void *value, uint64_t length)
{
	plist_t current = NULL;

	if (!plist)
		return NULL;

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

plist_t plist_find_node_by_key(plist_t plist, const char *value)
{
	return plist_find_node(plist, PLIST_KEY, value, strlen(value));
}

plist_t plist_find_node_by_string(plist_t plist, const char *value)
{
	return plist_find_node(plist, PLIST_STRING, value, strlen(value));
}

static void plist_get_type_and_value(plist_t node, plist_type * type, void *value, uint64_t * length)
{
	plist_data_t data = NULL;

	if (!node)
		return;

	data = plist_get_data(node);

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
	case PLIST_DATA:
		*((uint8_t **) value) = (uint8_t *) malloc(*length * sizeof(uint8_t));
		memcpy(*((uint8_t **) value), data->buff, *length * sizeof(uint8_t));
		break;
	case PLIST_DATE:
		//exception : here we use memory on the stack since it is just a temporary buffer
		(*((GTimeVal **) value))->tv_sec = data->timeval.tv_sec;
		(*((GTimeVal **) value))->tv_usec = data->timeval.tv_usec;
		break;
	case PLIST_ARRAY:
	case PLIST_DICT:
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

void plist_add_sub_node(plist_t node, plist_t subnode)
{
	if (node && subnode) {
		plist_type type = plist_get_node_type(node);
		if (type == PLIST_DICT || type == PLIST_ARRAY)
			g_node_append(node, subnode);
	}
}

void plist_add_sub_key_el(plist_t node, const char *val)
{
	plist_add_sub_element(node, PLIST_KEY, val, strlen(val));
}

void plist_add_sub_string_el(plist_t node, const char *val)
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

void plist_add_sub_data_el(plist_t node, const char *val, uint64_t length)
{
	plist_add_sub_element(node, PLIST_DATA, val, length);
}

void plist_add_sub_date_el(plist_t node, int32_t sec, int32_t usec)
{
	GTimeVal val = { sec, usec };
	plist_add_sub_element(node, PLIST_DATE, &val, sizeof(GTimeVal));
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
	if (PLIST_DATA == type)
		plist_get_type_and_value(node, &type, (void *) val, length);
}

void plist_get_date_val(plist_t node, int32_t * sec, int32_t * usec)
{
	plist_type type = plist_get_node_type(node);
	uint64_t length = 0;
	GTimeVal val = { 0, 0 };
	if (PLIST_DATE == type)
		plist_get_type_and_value(node, &type, (void *) &val, &length);
	assert(length == sizeof(GTimeVal));
	*sec = val.tv_sec;
	*usec = val.tv_usec;
}

gboolean plist_data_compare(gconstpointer a, gconstpointer b)
{
	plist_data_t val_a = NULL;
	plist_data_t val_b = NULL;

	if (!a || !b)
		return FALSE;

	if (!((GNode *) a)->data || !((GNode *) b)->data)
		return FALSE;

	val_a = plist_get_data((plist_t) a);
	val_b = plist_get_data((plist_t) b);

	if (val_a->type != val_b->type)
		return FALSE;

	switch (val_a->type) {
	case PLIST_BOOLEAN:
	case PLIST_UINT:
	case PLIST_REAL:
		if (val_a->intval == val_b->intval)	//it is an union so this is sufficient
			return TRUE;
		else
			return FALSE;

	case PLIST_KEY:
	case PLIST_STRING:
		if (!strcmp(val_a->strval, val_b->strval))
			return TRUE;
		else
			return FALSE;

	case PLIST_DATA:
		if (!memcmp(val_a->buff, val_b->buff, val_a->length))
			return TRUE;
		else
			return FALSE;
	case PLIST_ARRAY:
	case PLIST_DICT:
		//compare pointer
		if (a == b)
			return TRUE;
		else
			return FALSE;
		break;
	case PLIST_DATE:
		if (!memcmp(&(val_a->timeval), &(val_b->timeval), sizeof(GTimeVal)))
			return TRUE;
		else
			return FALSE;
	default:
		break;
	}
	return FALSE;
}

char plist_compare_node_value(plist_t node_l, plist_t node_r)
{
	return plist_data_compare(node_l, node_r);
}

static plist_t plist_set_element_val(plist_t node, plist_type type, const void *value, uint64_t length)
{
	//free previous allocated buffer
	plist_data_t data = plist_get_data(node);
	assert(data);				// a node should always have data attached

	switch (data->type) {
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
	case PLIST_DATA:
		data->buff = (uint8_t *) malloc(length);
		memcpy(data->buff, value, length);
		break;
	case PLIST_DATE:
		data->timeval.tv_sec = ((GTimeVal *) value)->tv_sec;
		data->timeval.tv_usec = ((GTimeVal *) value)->tv_usec;
		break;
	case PLIST_ARRAY:
	case PLIST_DICT:
	default:
		break;
	}
}


void plist_set_key_val(plist_t node, const char *val)
{
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
	plist_set_element_val(node, PLIST_UINT, &val, sizeof(uint64_t));
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
	GTimeVal val = { sec, usec };
	plist_set_element_val(node, PLIST_DATE, &val, sizeof(GTimeVal));
}
