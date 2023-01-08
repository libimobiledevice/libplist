/*
 * plist_otest.c
 * source libplist regression test
 *
 * Copyright (c) 2022 Nikias Bassen, All Rights Reserved.
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


#include "plist/plist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif


int main(int argc, char *argv[])
{
    FILE *iplist = NULL;
    plist_t root_node1 = NULL;
    plist_t root_node2 = NULL;
    char *plist_ostep = NULL;
    char *plist_ostep2 = NULL;
    char *plist_bin = NULL;
    int size_in = 0;
    uint32_t size_out = 0;
    uint32_t size_out2 = 0;
    char *file_in = NULL;
    char *file_out = NULL;
    struct stat filestats;
    if (argc != 3)
    {
        printf("Wrong input\n");
        return 1;
    }

    file_in = argv[1];
    file_out = argv[2];
    //read input file
    iplist = fopen(file_in, "rb");

    if (!iplist)
    {
        printf("File does not exists\n");
        return 2;
    }
    printf("File %s is open\n", file_in);
    stat(file_in, &filestats);
    size_in = filestats.st_size;
    plist_ostep = (char *) malloc(sizeof(char) * (size_in + 1));
    fread(plist_ostep, sizeof(char), size_in, iplist);
    fclose(iplist);
    plist_ostep[size_in] = 0;

    //convert one format to another
    plist_from_openstep(plist_ostep, size_in, &root_node1);
    if (!root_node1)
    {
        printf("OpenStep PList parsing failed\n");
        return 3;
    }

    printf("OpenStep PList parsing succeeded\n");
    plist_to_bin(root_node1, &plist_bin, &size_out);
    if (!plist_bin)
    {
        printf("PList BIN writing failed\n");
        return 4;
    }

    printf("PList BIN writing succeeded\n");
    plist_from_bin(plist_bin, size_out, &root_node2);
    if (!root_node2)
    {
        printf("PList BIN parsing failed\n");
        return 5;
    }

    printf("PList BIN parsing succeeded\n");
    plist_to_openstep(root_node2, &plist_ostep2, &size_out2, 0);
    if (!plist_ostep2)
    {
        printf("OpenStep PList writing failed\n");
        return 8;
    }

    printf("OpenStep PList writing succeeded\n");
    if (plist_ostep2)
    {
        FILE *oplist = NULL;
        oplist = fopen(file_out, "wb");
        fwrite(plist_ostep2, size_out2, sizeof(char), oplist);
        fclose(oplist);
    }

    plist_free(root_node1);
    plist_free(root_node2);
    free(plist_bin);
    free(plist_ostep);
    free(plist_ostep2);

    if ((uint32_t)size_in != size_out2)
    {
        printf("Size of input and output is different\n");
        printf("Input size : %i\n", size_in);
        printf("Output size : %i\n", size_out2);
    }

    //success
    return 0;
}

