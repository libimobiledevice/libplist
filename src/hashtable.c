/*
 * hashtable.c
 * really simple hash table implementation
 *
 * Copyright (c) 2011 Nikias Bassen, All Rights Reserved.
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
#include "hashtable.h"

hashtable_t* hash_table_new(hash_func_t hash_func, compare_func_t compare_func)
{
	hashtable_t* ht = (hashtable_t*)malloc(sizeof(hashtable_t));
	int i;
	for (i = 0; i < 256; i++) {
		ht->entries[i] = NULL;
	}
	ht->count = 0;
	ht->hash_func = hash_func;
	ht->compare_func = compare_func;
	return ht;
}

void hash_table_destroy(hashtable_t *ht)
{
	if (!ht) return;

	int i = 0;
	for (i = 0; i < 256; i++) {
		if (ht->entries[i]) {
			hashentry_t* e = ht->entries[i];
			while (e) {
				free(e->value);
				hashentry_t* old = e;
				e = (hashentry_t*)e->next;
				free(old);
			}
		}
	}
	free(ht);
}

void hash_table_insert(hashtable_t* ht, void *key, void *value)
{
	if (!ht || !key) return;

	unsigned int hash = ht->hash_func(key);

	int idx0 = hash & 0xFF;

	// get the idx0 list
	hashentry_t* e = ht->entries[idx0];
	while (e) {
		if (ht->compare_func(e->key, key)) {
			// element already present. replace value.
			e->value = value;
			return;
		}
		e = (hashentry_t*)e->next;
	}

	// if we get here, the element is not yet in the list.

	// make a new entry.
	hashentry_t* entry = (hashentry_t*)malloc(sizeof(hashentry_t));
	entry->key = key;
	entry->value = value;
	if (!ht->entries[idx0]) {
		// first entry
		entry->next = NULL;
	} else {
		// add to list
		entry->next = ht->entries[idx0];
	}
	ht->entries[idx0] = entry;
	ht->count++;
}

void* hash_table_lookup(hashtable_t* ht, void *key)
{
	if (!ht || !key) return NULL;
	unsigned int hash = ht->hash_func(key);

	int idx0 = hash & 0xFF;

	hashentry_t* e = ht->entries[idx0];
	while (e) {
		if (ht->compare_func(e->key, key)) {
			return e->value;
		}
		e = (hashentry_t*)e->next;
	}
	return NULL;
}
