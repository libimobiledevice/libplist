/*
 * Structure.cpp
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
#include <plist/Structure.h>

namespace PList
{

Structure::Structure(Node* parent) : Node(parent)
{
}
Structure::Structure(plist_type type, Node* parent) : Node(type, parent)
{
}

Structure::~Structure()
{
}

uint32_t Structure::GetSize()
{
    uint32_t size = 0;
    plist_type type = plist_get_node_type(_node);
    if (type == PLIST_ARRAY)
    {
        size = plist_array_get_size(_node);
    }
    else if (type == PLIST_DICT)
    {
        size = plist_dict_get_size(_node);
    }
    return size;
}

std::string Structure::ToXml()
{
    char* xml = NULL;
    uint32_t length = 0;
    plist_to_xml(_node, &xml, &length);
    std::string ret(xml, xml+length);
    free(xml);
    return ret;
}

std::vector<char> Structure::ToBin()
{
    char* bin = NULL;
    uint32_t length = 0;
    plist_to_bin(_node, &bin, &length);
    std::vector<char> ret(bin, bin+length);
    free(bin);
    return ret;
}

};

