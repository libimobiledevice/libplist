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

#include <algorithm>

namespace PList
{

Array::Array(Node* parent) : Structure(PLIST_ARRAY, parent)
{
    _array.clear();
}

Array::Array(plist_t node, Node* parent) : Structure(parent)
{
    _node = node;
    uint32_t size = plist_array_get_size(_node);

    for (uint32_t i = 0; i < size; i++)
    {
        plist_t subnode = plist_array_get_item(_node, i);
        _array.push_back(  Node::FromPlist(subnode, this) );
    }
}

Array::Array(PList::Array& a) : Structure()
{
    _array.clear();
    _node = plist_copy(a.GetPlist());
    uint32_t size = plist_array_get_size(_node);

    for (uint32_t i = 0; i < size; i++)
    {
        plist_t subnode = plist_array_get_item(_node, i);
        _array.push_back(  Node::FromPlist(subnode, this) );
    }
}

Array& Array::operator=(PList::Array& a)
{
    plist_free(_node);
    for (unsigned int it = 0; it < _array.size(); it++)
    {
        delete _array.at(it);
    }
    _array.clear();

    _node = plist_copy(a.GetPlist());
    uint32_t size = plist_array_get_size(_node);

    for (uint32_t i = 0; i < size; i++)
    {
        plist_t subnode = plist_array_get_item(_node, i);
        _array.push_back(  Node::FromPlist(subnode, this) );
    }
    return *this;
}

Array::~Array()
{
    for (unsigned int it = 0; it < _array.size(); it++)
    {
        delete (_array.at(it));
    }
    _array.clear();
}

Node* Array::Clone()
{
    return new Array(*this);
}

Node* Array::operator[](unsigned int index)
{
    return _array.at(index);
}

void Array::Append(Node* node)
{
    if (node)
    {
        Node* clone = node->Clone();
        UpdateNodeParent(clone);
        plist_array_append_item(_node, clone->GetPlist());
        _array.push_back(clone);
    }
}

void Array::Insert(Node* node, unsigned int pos)
{
    if (node)
    {
        Node* clone = node->Clone();
        UpdateNodeParent(clone);
        plist_array_insert_item(_node, clone->GetPlist(), pos);
        std::vector<Node*>::iterator it = _array.begin();
        it += pos;
        _array.insert(it, clone);
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

unsigned int Array::GetNodeIndex(Node* node)
{
    std::vector<Node*>::iterator it = std::find(_array.begin(), _array.end(), node);
    return std::distance (_array.begin(), it);
}

};
