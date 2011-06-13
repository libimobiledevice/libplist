/*
 * iterator.c
 *
 *  Created on: Mar 8, 2011
 *      Author: posixninja
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "object.h"
#include "iterator.h"

void iterator_destroy(iterator_t* iterator) {
	if(iterator) {
		free(iterator);
	}
}

iterator_t* iterator_create(list_t* list) {
	iterator_t* iterator = (iterator_t*) malloc(sizeof(iterator_t));
	if(iterator == NULL) {
		return NULL;
	}
	memset(iterator, '\0', sizeof(iterator_t));

	if(list != NULL) {
		// Create and bind to list

	} else {
		// Empty Iterator
	}

	return iterator;
}

object_t* iterator_next(iterator_t* iterator) {
	return NULL;
}

int iterator_bind(iterator_t* iterator, list_t* list) {
	return -1;
}
