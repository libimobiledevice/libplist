/*
 * Utils.cpp
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
#include <plist/Utils.h>
#include <plist/Dictionary.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Integer.h>
#include <plist/Real.h>
#include <plist/String.h>
#include <plist/Data.h>
#include <plist/Date.h>

namespace PList
{

Node* Utils::FromPlist(plist_t node, Node* parent)
{
    Node* ret = NULL;
    if (node)
    {
        plist_type type = plist_get_node_type(node);
        switch (type)
        {
        case PLIST_DICT:
            ret = new Dictionary(node, parent);
            break;
        case PLIST_ARRAY:
            ret = new Array(node, parent);
            break;
        case PLIST_BOOLEAN:
            ret = new Boolean(node, parent);
            break;
        case PLIST_UINT:
            ret = new Integer(node, parent);
            break;
        case PLIST_REAL:
            ret = new Real(node, parent);
            break;
        case PLIST_STRING:
            ret = new String(node, parent);
            break;
        case PLIST_DATE:
            ret = new Date(node, parent);
            break;
        case PLIST_DATA:
            ret = new Data(node, parent);
            break;
        default:
            plist_free(node);
            break;
        }
    }
    return ret;
}

static Structure* ImportStruct(plist_t root)
{
    Structure* ret = NULL;
    plist_type type = plist_get_node_type(root);

    if (PLIST_ARRAY == type || PLIST_DICT == type)
    {
        ret = static_cast<Structure*>(Utils::FromPlist(root));
    }
    else
    {
        plist_free(root);
    }

    return ret;
}

Structure* Utils::FromXml(const std::string& xml)
{
    plist_t root = NULL;
    plist_from_xml(xml.c_str(), xml.size(), &root);

    return ImportStruct(root);
}

Structure* Utils::FromBin(const std::vector<char>& bin)
{
    plist_t root = NULL;
    plist_from_bin(&bin[0], bin.size(), &root);

    return ImportStruct(root);

}

};
