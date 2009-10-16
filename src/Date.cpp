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

#include <stdlib.h>
#include <plist/Date.h>

namespace PList
{

Date::Date() : Node(PLIST_DATE)
{
}

Date::Date(plist_t node) : Node(node)
{
}

Date::Date(Date& d) : Node(PLIST_DATE)
{
    //TODO
}

Date& Date::operator=(PList::Date& b)
{
    //TODO
}

Date::Date(uint64_t i) : Node(PLIST_DATE)
{
    plist_set_date_val(_node, i, 0);
}

Date::~Date()
{
}

Node* Date::Clone()
{
    return new Date(*this);
}

void Date::SetValue(uint64_t i)
{
    plist_set_date_val(_node, i, 0);
}

uint64_t Date::GetValue()
{
    int32_t i = 0;
    plist_get_date_val(_node, &i, &i);
    return i;
}

};
