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

#include <cstdlib>
#include <algorithm>
#include <climits>
#include "plist.h"
#include <plist/Array.h>

namespace PList
{

Array::Array(Node* parent) : Structure(PLIST_ARRAY, parent)
{
    _array.clear();
}

static void array_fill(Array *_this, std::vector<Node*> &array, plist_t node)
{
    plist_array_iter iter = NULL;
    plist_array_new_iter(node, &iter);
    plist_t subnode;
    do {
        subnode = NULL;
        plist_array_next_item(node, iter, &subnode);
        if (subnode) {
            array.push_back( Node::FromPlist(subnode, _this) );
        }
    } while (subnode);
    free(iter);
}

Array::Array(plist_t node, Node* parent) : Structure(parent)
{
    _node = node;
    array_fill(this, _array, _node);
}

Array::Array(const PList::Array& a) : Structure(a.GetParent())
{
    _array.clear();
    _node = plist_copy(a.GetPlist());
    array_fill(this, _array, _node);
}

Array& Array::operator=(const PList::Array& a)
{
    plist_free(_node);
    for (size_t it = 0; it < _array.size(); it++) {
        delete _array.at(it);
    }
    _array.clear();
    _node = plist_copy(a.GetPlist());
    array_fill(this, _array, _node);
    return *this;
}

Array::~Array()
{
    for (size_t it = 0; it < _array.size(); it++) {
        delete (_array.at(it));
    }
    _array.clear();
}

Node* Array::Clone() const
{
    return new Array(*this);
}

Node* Array::operator[](unsigned int array_index)
{
    return _array.at(array_index);
}

Node* Array::Back()
{
    return _array.back();
}

Node* Array::back()
{
    return _array.back();
}

Node* Array::Front()
{
    return _array.front();
}

Node* Array::front()
{
    return _array.front();
}

Array::iterator Array::Begin()
{
    return _array.begin();
}

Array::iterator Array::begin()
{
    return _array.begin();
}

Array::iterator Array::End()
{
    return _array.end();
}

Array::iterator Array::end()
{
    return _array.end();
}

Array::const_iterator Array::Begin() const
{
    return _array.begin();
}

Array::const_iterator Array::begin() const
{
    return _array.begin();
}

Array::const_iterator Array::End() const
{
    return _array.end();
}

Array::const_iterator Array::end() const
{
    return _array.end();
}

size_t Array::size() const {
    return _array.size();
}

void Array::Append(const Node* node)
{
    if (node)
    {
        Node* clone = node->Clone();
        UpdateNodeParent(clone);
        plist_array_append_item(_node, clone->GetPlist());
        _array.push_back(clone);
    }
}

void Array::Append(const Node& node)
{
    Append(&node);
}

void Array::Insert(const Node* node, unsigned int pos)
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

void Array::Insert(const Node &node, unsigned int pos)
{
    Insert(&node, pos);
}

void Array::Remove(Node* node)
{
    if (node)
    {
        uint32_t pos = plist_array_get_item_index(node->GetPlist());
        if (pos == UINT_MAX) {
            return;
        }
        plist_array_remove_item(_node, pos);
        std::vector<Node*>::iterator it = _array.begin();
        it += pos;
        _array.erase(it);
        free(node);
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

unsigned int Array::GetNodeIndex(const Node* node) const
{
    std::vector<Node*>::const_iterator it = std::find(_array.begin(), _array.end(), node);
    return std::distance (_array.begin(), it);
}

unsigned int Array::GetNodeIndex(const Node& node) const
{
    return GetNodeIndex(&node);
}

}  // namespace PList
