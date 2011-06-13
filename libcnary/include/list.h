/*
 * list.h
 *
 *  Created on: Mar 8, 2011
 *      Author: posixninja
 */

#ifndef LIST_H_
#define LIST_H_

#include "object.h"

typedef struct list_t {
	void* next;
	void* prev;
} list_t;

void list_init(struct list_t* list);
void list_destroy(struct list_t* list);

int list_add(struct list_t* list, struct object_t* object);
int list_remove(struct list_t* list, struct object_t* object);

#endif /* LIST_H_ */
