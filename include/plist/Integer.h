/*
 * Integer.h
 * Integer node type for C++ binding
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

#ifndef PLIST_INTEGER_H
#define PLIST_INTEGER_H

#include <plist/Node.h>

namespace PList
{

class Integer : public Node
{
public :
    Integer(Node* parent = NULL);
    Integer(plist_t node, Node* parent = NULL);
    Integer(const Integer& i);
    Integer& operator=(const Integer& i);
    Integer(uint64_t i);
    Integer(int64_t i);
    virtual ~Integer();

    Node* Clone() const;

    void SetValue(int64_t i);
    void SetValue(uint64_t i);
    void SetUnsignedValue(uint64_t i);
    int64_t GetValue() const;
    uint64_t GetUnsignedValue() const;

    bool isNegative() const;
};

};

#endif // PLIST_INTEGER_H
