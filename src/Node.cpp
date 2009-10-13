/*
 * Node.cpp
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
#include <plist/Node.h>

namespace PList
{

Node::Node()
{
}

Node::Node(plist_t node) : _node(node)
{
}

Node::Node(plist_type type)
{
    _node = NULL;
    
    switch(type) {
	case PLIST_BOOLEAN:
	    _node = plist_new_bool(0);
	    break;
	case PLIST_UINT:
	    _node = plist_new_uint(0);
	    break;
	case PLIST_REAL:
	    _node = plist_new_real(0.);
	    break;
	case PLIST_STRING:
	    _node = plist_new_string("");
	    break;
	case PLIST_DATA:
	    _node = plist_new_data(NULL,0);
	    break;
	case PLIST_DATE:
	    _node = plist_new_date(0,0);
	    break;
	case PLIST_ARRAY:
	    _node = plist_new_array();
	    break;
	case PLIST_DICT:
	    _node = plist_new_dict();
	    break;
	case PLIST_KEY:
	case PLIST_NONE:
	default:
		break;
    }
}

Node::~Node()
{
    plist_free(_node);
    _node = NULL;
}

Node::Node(Node& node)
{
    plist_free(_node);
    _node = NULL;

    _node = plist_copy(_node);
}

Node& Node::operator=(const Node& node)
{
    plist_free(_node);
    _node = NULL;

    _node = plist_copy(_node);
}

plist_type Node::GetType()
{
    if (_node)
    {
	return plist_get_node_type(_node);
    }
}

plist_t Node::GetPlist() const
{
    return _node;
}
};
