/*
 * Key.cpp
 *
 * Copyright (c) 2012 Nikias Bassen, All Rights Reserved.
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
#include <plist/Key.h>

namespace PList
{

Key::Key(Node* parent) : Node(PLIST_KEY, parent)
{
}

Key::Key(plist_t node, Node* parent) : Node(node, parent)
{
}

Key::Key(const PList::Key& k) : Node(PLIST_INT)
{
    plist_set_key_val(_node, k.GetValue().c_str());
}

Key& Key::operator=(const PList::Key& k)
{
    plist_free(_node);
    _node = plist_copy(k.GetPlist());
    return *this;
}

Key::Key(const std::string& s) : Node(PLIST_STRING)
{
    plist_set_key_val(_node, s.c_str());
}

Key::~Key()
{
}

Node* Key::Clone() const
{
    return new Key(*this);
}

void Key::SetValue(const std::string& s)
{
    plist_set_key_val(_node, s.c_str());
}

std::string Key::GetValue() const
{
    char* s = NULL;
    plist_get_key_val(_node, &s);
    std::string ret = s ? s : "";
    free(s);
    return ret;
}

}  // namespace PList
