/*
 * node_list.c
 *
 *  Created on: Mar 8, 2011
 *      Author: posixninja
 *
 * Copyright (c) 2011 Joshua Hill. All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "node_list.h"

void node_list_destroy(node_list_t list)
{
	free(list);
}

node_list_t node_list_create()
{
	node_list_t list = (node_list_t)calloc(1, sizeof(struct node_list));
	if (list == NULL) {
		return NULL;
	}

	// Initialize structure
	list->begin = NULL;
	list->end = NULL;
	list->count = 0;
	return list;
}

int node_list_add(node_list_t list, node_t node)
{
	if (!list || !node) return NODE_ERR_INVALID_ARG;

	// Find the last element in the list
	node_t last = list->end;

	// Setup our new node as the new last element
	node->next = NULL;
	node->prev = last;

	// Set the next element of our old "last" element
	if (last) {
		// but only if the node list is not empty
		last->next = node;
	} else {
		// otherwise this is the start of the list
		list->begin = node;
	}

	// Set the lists prev to the new last element
	list->end = node;

	// Increment our node count for this list
	list->count++;
	return NODE_ERR_SUCCESS;
}

int node_list_insert(node_list_t list, unsigned int node_index, node_t node)
{
	if (!list || !node) return NODE_ERR_INVALID_ARG;
	if (node_index > list->count) return NODE_ERR_INVALID_ARG;
	if (node_index == list->count) {
		return node_list_add(list, node);
	}

	// Get the first element in the list
	node_t cur = list->begin;
	node_t prev = NULL;

	for (unsigned int pos = 0; pos < node_index; pos++) {
		if (!cur) return NODE_ERR_INVALID_ARG;
		prev = cur;
		cur = cur->next;
	}

	// insert node before cur
	node->prev = prev;
	node->next = cur;

	if (prev) {
		prev->next = node;
	} else {
		list->begin = node;
	}

	if (cur) {
		cur->prev = node;
	} else {
		// should not happen with bounds above, but keeps things consistent
		list->end = node;
	}

	// Increment our node count for this list
	list->count++;
	return NODE_ERR_SUCCESS;
}

// Returns removed index (>=0) on success, or NODE_ERR_* (<0) on failure.
int node_list_remove(node_list_t list, node_t node)
{
	if (!list || !node) return NODE_ERR_INVALID_ARG;
	if (list->count == 0) return NODE_ERR_NOT_FOUND;

	int node_index = 0;
	for (node_t n = list->begin; n; n = n->next, node_index++) {
		if (node != n) continue;

		node_t newnode = node->next;
		if (node->prev) {
			node->prev->next = newnode;
		} else {
			// we just removed the first element
			list->begin = newnode;
		}

		if (newnode) {
			newnode->prev = node->prev;
		} else {
			// we removed the last element, set new end
			list->end = node->prev;
		}

		// fully detach node from list
		node->prev = NULL;
		node->next = NULL;

		list->count--;
		return node_index;
	}
	return NODE_ERR_NOT_FOUND;
}
