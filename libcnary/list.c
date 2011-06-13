/*
 * list.c
 *
 *  Created on: Mar 8, 2011
 *      Author: posixninja
 */

#include <stdio.h>
#include <stdlib.h>

#include "list.h"

void list_init(list_t* list) {
	list->next = NULL;
	list->prev = list;
}


void list_destroy(list_t* list) {
	if(list) {
		free(list);
	}
}

int list_add(list_t* list, object_t* object) {
	return -1;
}

int list_remove(list_t* list, object_t* object) {
	return -1;
}
