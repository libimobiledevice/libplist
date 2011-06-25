/*
 * iterator.h
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

#ifndef ITERATOR_H_
#define ITERATOR_H_

struct list_t;
struct object_t;

typedef struct iterator_t {
	struct object_t*(*next)(struct iterator_t* iterator);
	int(*bind)(struct iterator_t* iterator, struct list_t* list);

	unsigned int count;
	unsigned int position;

	struct list_t* list;
	struct object_t* end;
	struct object_t* begin;
	struct object_t* value;
} iterator_t;

void iterator_destroy(struct iterator_t* iterator);
struct iterator_t* iterator_create(struct list_t* list);

struct object_t* iterator_next(struct iterator_t* iterator);
int iterator_bind(struct iterator_t* iterator, struct list_t* list);

#endif /* ITERATOR_H_ */
