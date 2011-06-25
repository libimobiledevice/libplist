/*
 * iterator.c
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
