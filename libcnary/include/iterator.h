/*
 * iterator.h
 *
 *  Created on: Mar 8, 2011
 *      Author: posixninja
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
