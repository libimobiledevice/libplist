/*
 * Array.h
 * Array node type for C++ binding
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

#ifndef PLIST_ARRAY_H
#define PLIST_ARRAY_H

#include <plist/Structure.h>
#include <vector>

namespace PList
{

class Array : public Structure
{
public :
    Array(Node* parent = NULL);
    Array(plist_t node, Node* parent = NULL);
    Array(const Array& a);
    Array& operator=(const Array& a);
    virtual ~Array();

    Node* Clone() const;

    typedef std::vector<Node*>::iterator iterator;
    typedef std::vector<Node*>::const_iterator const_iterator;

    Node* operator[](unsigned int index);
    Node* Back();
    Node* back();
    Node* Front();
    Node* front();
    iterator Begin();
    iterator begin();
    iterator End();
    iterator end();
    const_iterator Begin() const;
    const_iterator begin() const;
    const_iterator End() const;
    const_iterator end() const;
    size_t size() const;
    void Append(const Node& node);
    void Append(const Node* node);
    void Insert(const Node& node, unsigned int pos);
    void Insert(const Node* node, unsigned int pos);
    void Remove(Node* node);
    void Remove(unsigned int pos);
    unsigned int GetNodeIndex(const Node& node) const;
    unsigned int GetNodeIndex(const Node* node) const;
    template <typename T> T* at(unsigned int index) {
        return (T*)(_array.at(index));
    }
    template <typename T> T* At(unsigned int index) {
        return (T*)(_array.at(index));
    }

private :
    std::vector<Node*> _array;
};

};

#endif // PLIST_ARRAY_H
