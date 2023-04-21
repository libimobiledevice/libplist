/*
 * source libplist++ regression test
 *
 * Copyright (c) 2021 Sebastien Gonzalve All Rights Reserved.
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


#include "plist/plist++.h"
#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Wrong input\n");
        return 1;
    }

    const char* file_in = argv[1];
    const char* file_out = argv[2];

    //read input file
    std::ifstream iplist;
    iplist.open(file_in);

    if (!iplist)
    {
        printf("File does not exists\n");
        return 2;
    }

    std::cout << "File " << file_in << " is open\n";

    std::string plist_xml;
    {
        std::stringstream buffer;
        buffer << iplist.rdbuf();
        plist_xml = buffer.str();
    }

    iplist.close();

    //convert one format to another
    PList::Structure* root_node1 = PList::Structure::FromXml(plist_xml);
    if (!root_node1)
    {
        std::cout << "PList XML parsing failed\n";
        return 3;
    }

    std::cout << "PList XML parsing succeeded\n";
    std::vector<char> plist_bin = root_node1->ToBin();
    // FIXME There is no way to test for success of ToBin for now.

    std::cout << "PList BIN writing succeeded\n";
    PList::Structure* root_node2 = PList::Structure::FromBin(plist_bin);
    if (!root_node2)
    {
        std::cout << "PList BIN parsing failed\n";
        return 5;
    }

    std::cout << "PList BIN parsing succeeded\n";
    std::string plist_xml2 = root_node2->ToXml();
    if (plist_xml2.empty())
    {
        std::cout << "PList XML writing failed\n";
        return 8;
    }

    std::cout << "PList XML writing succeeded\n";
    {
        std::ofstream oplist;
        oplist.open(file_out);
        oplist << plist_xml2;
    }

    if (plist_xml.size() != plist_xml2.size())
    {
        std::cout << "Size of input and output is different\n"
            << "Input size : " << plist_xml.size()
            << "\nOutput size : " << plist_xml2.size() << '\n';
    }

    return 0;
}

