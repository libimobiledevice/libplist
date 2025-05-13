/*
 * Date.cpp
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
#include <plist/Date.h>

namespace PList
{

Date::Date(Node* parent) : Node(PLIST_DATE, parent)
{
}

Date::Date(plist_t node, Node* parent) : Node(node, parent)
{
}

Date::Date(const PList::Date& d) : Node(PLIST_DATE)
{
    int64_t t = d.GetValue();
    plist_set_unix_date_val(_node, t);
}

Date& Date::operator=(const PList::Date& d)
{
    plist_free(_node);
    _node = plist_copy(d.GetPlist());
    return *this;
}

Date::Date(int64_t t) : Node(PLIST_DATE)
{
    plist_set_unix_date_val(_node, t);
}

Date::~Date()
{
}

Node* Date::Clone() const
{
    return new Date(*this);
}

void Date::SetValue(int64_t t)
{
    plist_set_unix_date_val(_node, t);
}

int64_t Date::GetValue() const
{
    int64_t sec = 0;
    plist_get_unix_date_val(_node, &sec);
    return sec;
}

}  // namespace PList
