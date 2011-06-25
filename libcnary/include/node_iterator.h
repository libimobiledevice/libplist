/*
 * node_iterator.h
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

#ifndef NODE_ITERATOR_H_
#define NODE_ITERATOR_H_

#include "iterator.h"
#include "node_list.h"

// This class implements the abstract iterator class
typedef struct node_iterator_t {
	// Super class
	struct iterator_t super;

	// Local members
	struct node_t*(*next)(struct node_iterator_t* iterator);
	int(*bind)(struct node_iterator_t* iterator, struct node_list_t* list);

	unsigned int count;
	unsigned int position;

	struct node_list_t* list;
	struct node_t* end;
	struct node_t* begin;
	struct node_t* value;

} node_iterator_t;

void node_iterator_destroy(node_iterator_t* iterator);
node_iterator_t* node_iterator_create(node_list_t* list);

struct node_t* node_iterator_next(struct node_iterator_t* iterator);
int node_iterator_bind(struct node_iterator_t* iterator, struct node_list_t* list);

#endif /* NODE_ITERATOR_H_ */
