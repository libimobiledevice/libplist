/*
 * backup_test.c
 * source libplist regression test
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


#include "plist/plist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <node.h>

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

static plist_t plist_get_first_child(plist_t node)
{
    return (plist_t) node_first_child((node_t) node);
}

static plist_t plist_get_next_sibling(plist_t node)
{
    return (plist_t) node_next_sibling((node_t) node);
}

static char compare_plist(plist_t node_l, plist_t node_r)
{
    plist_t cur_l = NULL;
    plist_t cur_r = NULL;
    int res = 1;

    cur_l = plist_get_first_child(node_l);
    cur_r = plist_get_first_child(node_r);

    if ( (!cur_l && cur_r) || (cur_l && !cur_r))
        return 0;

    if ( !cur_l && !cur_r )
        return plist_compare_node_value( node_l, node_r );

    while (cur_l && cur_r && res)
    {

        if (!(res = compare_plist(cur_l, cur_r)))
            return res;

        cur_l = plist_get_next_sibling(cur_l);
        cur_r = plist_get_next_sibling(cur_r);
        if ( (!cur_l && cur_r) || (cur_l && !cur_r))
            return 0;
    }

    return res;
}

int main(int argc, char *argv[])
{
    plist_t root_node1 = NULL;
    plist_t root_node2 = NULL;
    char *file_in1 = NULL;
    char *file_in2 = NULL;
    int res = 0;

    if (argc!= 3)
    {
        printf("Wrong input\n");
        return 1;
    }

    file_in1 = argv[1];
    file_in2 = argv[2];

    plist_read_from_file(file_in1, &root_node1, NULL);
    plist_read_from_file(file_in2, &root_node2, NULL);

    if (!root_node1 || !root_node2)
    {
        printf("PList parsing failed\n");
        return 3;
    }

    printf("PList parsing succeeded\n");
    res = compare_plist(root_node1, root_node2);

    plist_free(root_node1);
    plist_free(root_node2);

    return !res;
}

