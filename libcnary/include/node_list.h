/*
 * node_list.h
 *
 *  Created on: Mar 8, 2011
 *      Author: posixninja
 */

#ifndef NODE_LIST_H_
#define NODE_LIST_H_

struct node_t;

// This class implements the list_t abstract class
typedef struct node_list_t {
	// list_t members
	struct node_t* begin;
	struct node_t* end;

	// node_list_t members
	unsigned int count;

} node_list_t;

void node_list_destroy(struct node_list_t* list);
struct node_list_t* node_list_create(struct node_t* node);

int node_list_add(node_list_t* list, node_t* node);
int node_list_insert(node_list_t* list, unsigned int index, node_t* node);
int node_list_remove(node_list_t* list, node_t* node);

#endif /* NODE_LIST_H_ */
