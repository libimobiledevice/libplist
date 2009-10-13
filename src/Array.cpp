/*
 * Array.cpp
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
#include <plist/Array.h>
#include <plist/Dictionary.h>

namespace PList
{

Array::Array() : Structure(PLIST_ARRAY)
{
    _array.clear();
}

Array::Array(plist_t node) : Structure()
{
    _node = node;
    uint32_t size = plist_array_get_size(_node);

    for (uint32_t i = 0; i < size; i++)
    {
	plist_t subnode = plist_array_get_item(_node, i);
	plist_type subtype = plist_get_node_type(subnode);
	switch(subtype)
	{
	    case PLIST_DICT:
		_array.push_back( new Dictionary(subnode) );
		break;
	    case PLIST_ARRAY:
		_array.push_back( new Array(subnode) );
		break;
	    case PLIST_BOOLEAN:
	    case PLIST_UINT:
	    case PLIST_REAL:
	    case PLIST_STRING:
	    case PLIST_DATE:
	    case PLIST_DATA:
	    default:
		_array.push_back( new Node(subnode) );
		break;
	}
    }
}

Array::Array(Array& a)
{
    plist_free(_node);
    for (int it = 0; it < _array.size(); it++)
    {
	delete _array.at(it);
    }
    _array.clear();
    
    _node = plist_copy(a.GetPlist());
    uint32_t size = plist_array_get_size(_node);

    for (uint32_t i = 0; i < size; i++)
    {
	plist_t subnode = plist_array_get_item(_node, i);
	plist_type subtype = plist_get_node_type(subnode);
	switch(subtype)
	{
	    case PLIST_DICT:
		_array.push_back( new Dictionary(subnode) );
		break;
	    case PLIST_ARRAY:
		_array.push_back( new Array(subnode) );
		break;
	    case PLIST_BOOLEAN:
	    case PLIST_UINT:
	    case PLIST_REAL:
	    case PLIST_STRING:
	    case PLIST_DATE:
	    case PLIST_DATA:
	    default:
		_array.push_back( new Node(subnode) );
		break;
	}
    }
}

Array& Array::operator=(const Array& a)
{
    plist_free(_node);
    for (int it = 0; it < _array.size(); it++)
    {
	delete _array.at(it);
    }
    _array.clear();

    _node = plist_copy(a.GetPlist());
    uint32_t size = plist_array_get_size(_node);

    for (uint32_t i = 0; i < size; i++)
    {
	plist_t subnode = plist_array_get_item(_node, i);
	plist_type subtype = plist_get_node_type(subnode);
	switch(subtype)
	{
	    case PLIST_DICT:
		_array.push_back( new Dictionary(subnode) );
		break;
	    case PLIST_ARRAY:
		_array.push_back( new Array(subnode) );
		break;
	    case PLIST_BOOLEAN:
	    case PLIST_UINT:
	    case PLIST_REAL:
	    case PLIST_STRING:
	    case PLIST_DATE:
	    case PLIST_DATA:
	    default:
		_array.push_back( new Node(subnode) );
		break;
	}
    }
}

Array::~Array()
{
    plist_free(_node);
    for (int it = 0; it < _array.size(); it++)
    {
	delete _array.at(it);
    }
    _array.clear();
}

Node* Array::operator[](unsigned int index)
{
    return _array.at(index);
}

void Array::Append(Node* node)
{
    if (node)
    {
	plist_array_append_item(_node, node->GetPlist());
	_array.push_back(node);
    }
}

void Array::Insert(Node* node, unsigned int pos)
{
    if (node)
    {
	plist_array_insert_item(_node, node->GetPlist(), pos);
	std::vector<Node*>::iterator it = _array.begin();
	it += pos;
	_array.insert(it, node);
    }
}

void Array::Remove(Node* node)
{
    if (node)
    {
	uint32_t pos = plist_array_get_item_index(node->GetPlist());
	plist_array_remove_item(_node, pos);
	std::vector<Node*>::iterator it = _array.begin();
	it += pos;
	_array.erase(it);
	delete node;
    }
}

void Array::Remove(unsigned int pos)
{
    plist_array_remove_item(_node, pos);
    std::vector<Node*>::iterator it = _array.begin();
    it += pos;
    delete _array.at(pos);
    _array.erase(it);
}

};
