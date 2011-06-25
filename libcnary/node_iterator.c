/*
 * node_iterator.c
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
#include "node_iterator.h"

void node_iterator_destroy(node_iterator_t* iterator) {
	if(iterator) {
		free(iterator);
	}
}

node_iterator_t* node_iterator_create(node_list_t* list) {
	node_iterator_t* iterator = (node_iterator_t*) malloc(sizeof(node_iterator_t));
	if(iterator == NULL) {
		return NULL;
	}
	memset(iterator, '\0', sizeof(node_iterator_t));

	iterator->count = 0;
	iterator->position = 0;

	iterator->end = NULL;
	iterator->begin = NULL;
	iterator->value = list->begin;

	iterator->list = NULL;
	iterator->next = node_iterator_next;
	iterator->bind = node_iterator_bind;


	if(list != NULL) {
		iterator->bind(iterator, list);
	}

	return iterator;
}

node_t* node_iterator_next(node_iterator_t* iterator) {
	node_t* node = iterator->value;
	if (node) {
		iterator->value = node->next;
	}
	iterator->position++;
	return node;
}

int node_iterator_bind(node_iterator_t* iterator, node_list_t* list) {
	iterator->position = 0;
	iterator->end = list->end;
	iterator->count = list->count;
	iterator->begin = list->begin;
	iterator->value = list->begin;
	return 0;
}
