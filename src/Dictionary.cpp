/*
 * Dictionary.cpp
 *
 * Copyright (c) 2009 Jonathan Beck All Rights Reserved.
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

#include <stdlib.h>
#include <plist/Dictionary.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Integer.h>
#include <plist/Real.h>
#include <plist/String.h>
#include <plist/Date.h>
#include <plist/Data.h>

namespace PList
{

Dictionary::Dictionary() : Structure(PLIST_DICT)
{
}

Dictionary::Dictionary(plist_t node) : Structure()
{
    _node = node;
    plist_dict_iter it = NULL;

    char* key = NULL;
    plist_t subnode = NULL;
    plist_dict_new_iter(_node, &it);
    plist_dict_next_item(_node, it, &key, &subnode);
    while (subnode)
    {
	plist_type subtype = plist_get_node_type(subnode);
	switch(subtype)
	{
	    case PLIST_DICT:
		_map[std::string(key)] = new Dictionary(subnode);
		break;
	    case PLIST_ARRAY:
		_map[std::string(key)] = new Array(subnode);
		break;
	    case PLIST_BOOLEAN:
		_map[std::string(key)] = new Boolean(subnode);
		break;
	    case PLIST_UINT:
		_map[std::string(key)] = new Integer(subnode);
		break;
	    case PLIST_REAL:
		_map[std::string(key)] = new Real(subnode);
		break;
	    case PLIST_STRING:
		_map[std::string(key)] = new String(subnode);
		break;
	    case PLIST_DATE:
		_map[std::string(key)] = new Date(subnode);
		break;
	    case PLIST_DATA:
		_map[std::string(key)] = new Data(subnode);
		break;
	    default:
		break;
	}
	
	subnode = NULL;
	free(key);
	key = NULL;
	plist_dict_next_item(_node, it, &key, &subnode);
    }
    free(it);
}

Dictionary::Dictionary(PList::Dictionary& d) : Structure()
{
    for (Dictionary::iterator it = _map.begin(); it != _map.end(); it++)
    {
	plist_free(it->second->GetPlist());
	delete it->second;
    }
    _map.clear();

    _node = plist_copy(d.GetPlist());
    plist_dict_iter it = NULL;

    char* key = NULL;
    plist_t subnode = NULL;
    plist_dict_new_iter(_node, &it);
    plist_dict_next_item(_node, it, &key, &subnode);
    while (subnode)
    {
	plist_type subtype = plist_get_node_type(subnode);
	switch(subtype)
	{
	    case PLIST_DICT:
		_map[std::string(key)] = new Dictionary(subnode);
		break;
	    case PLIST_ARRAY:
		_map[std::string(key)] = new Array(subnode);
		break;
	    case PLIST_BOOLEAN:
		_map[std::string(key)] = new Boolean(subnode);
		break;
	    case PLIST_UINT:
		_map[std::string(key)] = new Integer(subnode);
		break;
	    case PLIST_REAL:
		_map[std::string(key)] = new Real(subnode);
		break;
	    case PLIST_STRING:
		_map[std::string(key)] = new String(subnode);
		break;
	    case PLIST_DATE:
		_map[std::string(key)] = new Date(subnode);
		break;
	    case PLIST_DATA:
		_map[std::string(key)] = new Data(subnode);
		break;
	    default:
		break;
	}
	
	subnode = NULL;
	free(key);
	key = NULL;
	plist_dict_next_item(_node, it, NULL, &subnode);
    }
    free(it);
}

Dictionary& Dictionary::operator=(PList::Dictionary& d)
{
    for (Dictionary::iterator it = _map.begin(); it != _map.end(); it++)
    {
	plist_free(it->second->GetPlist());
	delete it->second;
    }
    _map.clear();

    _node = plist_copy(d.GetPlist());
    plist_dict_iter it = NULL;

    char* key = NULL;
    plist_t subnode = NULL;
    plist_dict_new_iter(_node, &it);
    plist_dict_next_item(_node, it, &key, &subnode);
    while (subnode)
    {
	plist_type subtype = plist_get_node_type(subnode);
	switch(subtype)
	{
	    case PLIST_DICT:
		_map[std::string(key)] = new Dictionary(subnode);
		break;
	    case PLIST_ARRAY:
		_map[std::string(key)] = new Array(subnode);
		break;
	    case PLIST_BOOLEAN:
		_map[std::string(key)] = new Boolean(subnode);
		break;
	    case PLIST_UINT:
		_map[std::string(key)] = new Integer(subnode);
		break;
	    case PLIST_REAL:
		_map[std::string(key)] = new Real(subnode);
		break;
	    case PLIST_STRING:
		_map[std::string(key)] = new String(subnode);
		break;
	    case PLIST_DATE:
		_map[std::string(key)] = new Date(subnode);
		break;
	    case PLIST_DATA:
		_map[std::string(key)] = new Data(subnode);
		break;
	    default:
		break;
	}
	
	subnode = NULL;
	free(key);
	key = NULL;
	plist_dict_next_item(_node, it, NULL, &subnode);
    }
    free(it);
}

Dictionary::~Dictionary()
{
    for (Dictionary::iterator it = _map.begin(); it != _map.end(); it++)
    {
	plist_free(it->second->GetPlist());
	delete it->second;
    }
    _map.clear();
}

Node* Dictionary::Clone()
{
    return new Dictionary(*this);
}

Node* Dictionary::operator[](const std::string& key)
{
    return _map[key];
}

Dictionary::iterator Dictionary::Begin()
{
    return _map.begin();
}

Dictionary::iterator Dictionary::End()
{
    return _map.end();
}

void Dictionary::Insert(const std::string& key, Node* node)
{
    if (node)
    {
	Node* clone = node->Clone();
	plist_dict_insert_item(_node, key.c_str(), clone->GetPlist());
	delete _map[key];
	_map[key] = clone;
    }
}

void Dictionary::Remove(Node* node)
{
    if (node)
    {
	char* key = NULL;
	plist_dict_get_item_key(node->GetPlist(), &key);
	plist_dict_remove_item(_node, key);
	std::string skey = key;
	free(key);
	delete node;
    }
}

void Dictionary::Remove(const std::string& key)
{
	plist_dict_remove_item(_node, key.c_str());
	delete _map[key];
}

};
