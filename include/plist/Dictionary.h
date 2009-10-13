/*
 * Dictionary.h
 * Dictionary node type for C++ binding
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

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <plist/Structure.h>
#include <map>
#include <string>

namespace PList
{

class Dictionary : public Structure
{
    public :
	Dictionary();
	Dictionary(plist_t node);
	Dictionary(Dictionary& d);
	Dictionary& operator=(const Dictionary& d);
	virtual ~Dictionary();
	
	typedef std::map<std::string,Node*>::iterator iterator;
	
	Node* operator[](std::string& key);
	iterator Begin();
	iterator End();
	void Insert(std::string& key, Node* node);
	void Remove(Node* node);
	void Remove(std::string& key);
	
    private :
	std::map<std::string,Node*> _map;


};

};

#endif // DICTIONARY_H
