/*
 * plist.h
 * contains structures and the like for plists
 *
 * Copyright (c) 2008 Zach C. All Rights Reserved.
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

#ifndef PLIST_H
#define PLIST_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "plist/plist.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4244)
#endif

struct plist_data_s
{
    union
    {
        char boolval;
        uint64_t intval;
        double realval;
        char *strval;
        uint8_t *buff;
        void *hashtable;
    };
    uint64_t length;
    plist_type type;
};

typedef struct plist_data_s *plist_data_t;

plist_t plist_new_node(plist_data_t data);
plist_data_t plist_get_data(plist_t node);
plist_data_t plist_new_plist_data(void);
void plist_free_data(plist_data_t data);
int plist_data_compare(const void *a, const void *b);

extern plist_err_t plist_write_to_string_default(plist_t plist, char **output, uint32_t* length, plist_write_options_t options);
extern plist_err_t plist_write_to_string_limd(plist_t plist, char **output, uint32_t* length, plist_write_options_t options);
extern plist_err_t plist_write_to_string_plutil(plist_t plist, char **output, uint32_t* length, plist_write_options_t options);
extern plist_err_t plist_write_to_stream_default(plist_t plist, FILE *stream, plist_write_options_t options);
extern plist_err_t plist_write_to_stream_limd(plist_t plist, FILE *stream, plist_write_options_t options);
extern plist_err_t plist_write_to_stream_plutil(plist_t plist, FILE *stream, plist_write_options_t options);

#endif
