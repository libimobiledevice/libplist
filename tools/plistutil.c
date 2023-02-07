/*
 * plistutil.c
 * Simple tool to convert a plist into different formats
 *
 * Copyright (c) 2009-2020 Martin Szulecki All Rights Reserved.
 * Copyright (c) 2013-2020 Nikias Bassen, All Rights Reserved.
 * Copyright (c) 2008 Zach C., All Rights Reserved.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "plist/plist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

typedef struct _options
{
    char *in_file, *out_file;
    uint8_t in_fmt, out_fmt; // fmts 0 = undef, 1 = bin, 2 = xml, 3 = json, 4 = openstep
    uint8_t flags;
} options_t;
#define OPT_DEBUG   (1 << 0)
#define OPT_COMPACT (1 << 1)
#define OPT_SORT    (1 << 2)

static void print_usage(int argc, char *argv[])
{
    char *name = NULL;
    name = strrchr(argv[0], '/');
    printf("Usage: %s [OPTIONS] [-i FILE] [-o FILE]\n", (name ? name + 1: argv[0]));
    printf("\n");
    printf("Convert a plist FILE between binary, XML, JSON, and OpenStep format.\n");
    printf("If -f is omitted, XML plist data will be converted to binary and vice-versa.\n");
    printf("To convert to/from JSON or OpenStep the output format needs to be specified.\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -i, --infile FILE    Optional FILE to convert from or stdin if - or not used\n");
    printf("  -o, --outfile FILE   Optional FILE to convert to or stdout if - or not used\n");
    printf("  -f, --format FORMAT  Force output format, regardless of input type\n");
    printf("                       FORMAT is one of xml, bin, json, or openstep\n");
    printf("                       If omitted, XML will be converted to binary,\n");
    printf("                       and binary to XML.\n");
    printf("  -c, --compact        JSON and OpenStep only: Print output in compact form.\n");
    printf("                       By default, the output will be pretty-printed.\n");
    printf("  -s, --sort           Sort all dictionary nodes lexicographically by key\n");
    printf("                       before converting to the output format.\n");
    printf("  -d, --debug          Enable extended debug output\n");
    printf("  -v, --version        Print version information\n");
    printf("\n");
    printf("Homepage:    <" PACKAGE_URL ">\n");
    printf("Bug Reports: <" PACKAGE_BUGREPORT ">\n");
}

static options_t *parse_arguments(int argc, char *argv[])
{
    int i = 0;

    options_t *options = (options_t*)calloc(1, sizeof(options_t));
    options->out_fmt = 0;

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--infile") || !strcmp(argv[i], "-i"))
        {
            if ((i + 1) == argc)
            {
                free(options);
                return NULL;
            }
            options->in_file = argv[i + 1];
            i++;
            continue;
        }
        else if (!strcmp(argv[i], "--outfile") || !strcmp(argv[i], "-o"))
        {
            if ((i + 1) == argc)
            {
                free(options);
                return NULL;
            }
            options->out_file = argv[i + 1];
            i++;
            continue;
        }
        else if (!strcmp(argv[i], "--format") || !strcmp(argv[i], "-f"))
        {
            if ((i + 1) == argc)
            {
                free(options);
                return NULL;
            }
            if (!strncmp(argv[i+1], "bin", 3)) {
                options->out_fmt = 1;
            } else if (!strncmp(argv[i+1], "xml", 3)) {
                options->out_fmt = 2;
            } else if (!strncmp(argv[i+1], "json", 4)) {
                options->out_fmt = 3;
            } else if (!strncmp(argv[i+1], "openstep", 8) || !strncmp(argv[i+1], "ostep", 5)) {
                options->out_fmt = 4;
            } else {
                fprintf(stderr, "ERROR: Unsupported output format\n");
                free(options);
                return NULL;
            }
            i++;
            continue;
        }
        else if (!strcmp(argv[i], "--compact") || !strcmp(argv[i], "-c"))
        {
            options->flags |= OPT_COMPACT;
        }
        else if (!strcmp(argv[i], "--sort") || !strcmp(argv[i], "-s"))
        {
            options->flags |= OPT_SORT;
        }
        else if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d"))
        {
            options->flags |= OPT_DEBUG;
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
        {
            free(options);
            return NULL;
        }
        else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v"))
        {
            printf("plistutil %s\n", PACKAGE_VERSION);
            exit(EXIT_SUCCESS);
        }
        else
        {
            fprintf(stderr, "ERROR: Invalid option '%s'\n", argv[i]);
            free(options);
            return NULL;
        }
    }

    return options;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int input_res = PLIST_ERR_UNKNOWN;
    int output_res = PLIST_ERR_UNKNOWN;
    FILE *iplist = NULL;
    plist_t root_node = NULL;
    char *plist_out = NULL;
    uint32_t size = 0;
    int read_size = 0;
    int read_capacity = 4096;
    char *plist_entire = NULL;
    struct stat filestats;
    options_t *options = parse_arguments(argc, argv);

    if (!options)
    {
        print_usage(argc, argv);
        return 0;
    }

    if (options->flags & OPT_DEBUG)
    {
        plist_set_debug(1);
    }

    if (!options->in_file || !strcmp(options->in_file, "-"))
    {
        read_size = 0;
        plist_entire = malloc(sizeof(char) * read_capacity);
        if(plist_entire == NULL)
        {
            fprintf(stderr, "ERROR: Failed to allocate buffer to read from stdin");
            free(options);
            return 1;
        }
        plist_entire[read_size] = '\0';
        char ch;
        while(read(STDIN_FILENO, &ch, 1) > 0)
        {
            if (read_size >= read_capacity) {
                char *old = plist_entire;
                read_capacity += 4096;
                plist_entire = realloc(plist_entire, sizeof(char) * read_capacity);
                if (plist_entire == NULL)
                {
                    fprintf(stderr, "ERROR: Failed to reallocate stdin buffer\n");
                    free(old);
                    free(options);
                    return 1;
                }
            }
            plist_entire[read_size] = ch;
            read_size++;
        }
        if (read_size >= read_capacity) {
            char *old = plist_entire;
            plist_entire = realloc(plist_entire, sizeof(char) * (read_capacity+1));
            if (plist_entire == NULL)
            {
                fprintf(stderr, "ERROR: Failed to reallocate stdin buffer\n");
                free(old);
                free(options);
                return 1;
            }
        }
        plist_entire[read_size] = '\0';

        // Not positive we need this, but it doesnt seem to hurt lol
        if(ferror(stdin))
        {
            fprintf(stderr, "ERROR: reading from stdin.\n");
            free(plist_entire);
            free(options);
            return 1;
        }
    }
    else
    {
        // read input file
        iplist = fopen(options->in_file, "rb");
        if (!iplist) {
            fprintf(stderr, "ERROR: Could not open input file '%s': %s\n", options->in_file, strerror(errno));
            free(options);
            return 1;
        }

        memset(&filestats, '\0', sizeof(struct stat));
        fstat(fileno(iplist), &filestats);

        plist_entire = (char *) malloc(sizeof(char) * (filestats.st_size + 1));
        read_size = fread(plist_entire, sizeof(char), filestats.st_size, iplist);
        plist_entire[read_size] = '\0';
        fclose(iplist);
    }

    if (options->out_fmt == 0) {
        // convert from binary to xml or vice-versa
        if (plist_is_binary(plist_entire, read_size))
        {
            input_res = plist_from_bin(plist_entire, read_size, &root_node);
            if (input_res == PLIST_ERR_SUCCESS) {
                if (options->flags & OPT_SORT) {
                    plist_sort(root_node);
                }
                output_res = plist_to_xml(root_node, &plist_out, &size);
            }
        }
        else
        {
            input_res = plist_from_xml(plist_entire, read_size, &root_node);
            if (input_res == PLIST_ERR_SUCCESS) {
                if (options->flags & OPT_SORT) {
                    plist_sort(root_node);
                }
                output_res = plist_to_bin(root_node, &plist_out, &size);
            }
        }
    }
    else
    {
        input_res = plist_from_memory(plist_entire, read_size, &root_node);
        if (input_res == PLIST_ERR_SUCCESS) {
            if (options->flags & OPT_SORT) {
                plist_sort(root_node);
            }
            if (options->out_fmt == 1) {
                output_res = plist_to_bin(root_node, &plist_out, &size);
            } else if (options->out_fmt == 2) {
                output_res = plist_to_xml(root_node, &plist_out, &size);
            } else if (options->out_fmt == 3) {
                output_res = plist_to_json(root_node, &plist_out, &size, !(options->flags & OPT_COMPACT));
            } else if (options->out_fmt == 4) {
                output_res = plist_to_openstep(root_node, &plist_out, &size, !(options->flags & OPT_COMPACT));
            }
        }
    }
    plist_free(root_node);
    free(plist_entire);

    if (plist_out)
    {
        if (options->out_file != NULL && strcmp(options->out_file, "-") != 0)
        {
            FILE *oplist = fopen(options->out_file, "wb");
            if (!oplist) {
                fprintf(stderr, "ERROR: Could not open output file '%s': %s\n", options->out_file, strerror(errno));
                free(options);
                return 1;
            }
            fwrite(plist_out, size, sizeof(char), oplist);
            fclose(oplist);
        }
        // if no output file specified, write to stdout
        else
            fwrite(plist_out, size, sizeof(char), stdout);

        free(plist_out);
    }

    if (input_res == PLIST_ERR_SUCCESS) {
        switch (output_res) {
            case PLIST_ERR_SUCCESS:
                break;
            case PLIST_ERR_FORMAT:
                fprintf(stderr, "ERROR: Input plist data is not compatible with output format.\n");
                ret = 2;
                break;
            default:
                fprintf(stderr, "ERROR: Failed to convert plist data (%d)\n", output_res);
                ret = 1;
        }
    } else {
        switch (input_res) {
            case PLIST_ERR_PARSE:
                if (options->out_fmt == 0) {
                    fprintf(stderr, "ERROR: Could not parse plist data, expected XML or binary plist\n");
                } else {
                    fprintf(stderr, "ERROR: Could not parse plist data (%d)\n", input_res);
                }
                ret = 3;
                break;
            default:
                fprintf(stderr, "ERROR: Could not parse plist data (%d)\n", input_res);
                ret = 1;
                break;
        }
    }

    free(options);
    return ret;
}
