/*
 * String.cpp
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
#include "plist.h"
#include <plist/String.h>

namespace PList
{

String::String(Node* parent) : Node(PLIST_STRING, parent)
{
}

String::String(plist_t node, Node* parent) : Node(node, parent)
{
}

String::String(const PList::String& s) : Node(PLIST_INT)
{
    plist_set_string_val(_node, s.GetValue().c_str());
}

String& String::operator=(const PList::String& s)
{
    if (this == &s) return *this;

    plist_free(_node);
    _node = plist_copy(s.GetPlist());
    return *this;
}

String& String::operator=(const std::string& s)
{
    plist_free(_node);
    _node = plist_new_string(s.c_str());
    return *this;
}

String& String::operator=(const char* s)
{
    plist_free(_node);
    _node = plist_new_string(s);
    return *this;
}

String::String(const std::string& s) : Node(PLIST_STRING)
{
    plist_set_string_val(_node, s.c_str());
}

String::String(const char *s) : Node(PLIST_STRING)
{
    plist_set_string_val(_node, s);
}

String::~String()
{
}

Node* String::Clone() const
{
    return new String(*this);
}

void String::SetValue(const std::string& s)
{
    plist_set_string_val(_node, s.c_str());
}

std::string String::GetValue() const
{
    const char* s = plist_get_string_ptr(_node, NULL);
    std::string ret = s ? s : "";
    return ret;
}

}  // namespace PList
