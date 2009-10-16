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

namespace PList
{

static Structure* FromPlist(plist_t root)
{
    Structure* ret = NULL;
    if (root)
    {
	plist_type type = plist_get_node_type(root);
	switch(type)
	{
	    case PLIST_DICT:
		ret = new Dictionary(root);
		break;
	    case PLIST_ARRAY:
		ret = new Array(root);
		break;
	    case PLIST_BOOLEAN:
	    case PLIST_UINT:
	    case PLIST_REAL:
	    case PLIST_STRING:
	    case PLIST_DATE:
	    case PLIST_DATA:
	    default:
		plist_free(root);
		break;
	}
    }
    return ret;
}

Structure* Utils::FromXml(const std::string& in)
{
    plist_t root = NULL;
    plist_from_xml(in.c_str(), in.size(), &root);

    return FromPlist(root);
}

Structure* Utils::FromBin(const std::vector<char>& in)
{
    plist_t root = NULL;
    plist_from_bin(&in[0], in.size(), &root);

    return FromPlist(root);

}

};
