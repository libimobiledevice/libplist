/*
 * Node.h
 * Abstract node type for C++ binding
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

#ifndef NODE_H
#define NODE_H

#include <plist/plist.h>

namespace PList
{

class Node
{
    public :
	virtual ~Node();
	Node(plist_t node);
	Node(Node& node);
	Node& operator=(const Node& node);
	
	plist_type GetType();
	plist_t GetPlist() const;
	
    protected:
	Node();
	Node(plist_type type);
	plist_t _node;
};

};

#endif // NODE_H
