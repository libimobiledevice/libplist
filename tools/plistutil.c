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
#include <getopt.h>
#include <errno.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4996)
#define STDIN_FILENO _fileno(stdin)
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
    printf("  -p, --print FILE     Print the PList in human-readable format.\n");
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
    options_t *options = calloc(1, sizeof(options_t));
    if (!options)
        return NULL;

    options->out_fmt = 0;

    static struct option long_options[] = {
        { "infile",   required_argument, 0, 'i' },
        { "outfile",  required_argument, 0, 'o' },
        { "format",   required_argument, 0, 'f' },
        { "compact",  no_argument,       0, 'c' },
        { "sort",     no_argument,       0, 's' },
        { "print",    required_argument, 0, 'p' },
        { "debug",    no_argument,       0, 'd' },
        { "help",     no_argument,       0, 'h' },
        { "version",  no_argument,       0, 'v' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "i:o:f:csp:dhv", long_options, NULL)) != -1)
    {
        switch (c)
        {
            case 'i':
                if (!optarg || optarg[0] == '\0') {
                    fprintf(stderr, "ERROR: --infile requires a filename or '-' for stdin\n");
                    free(options);
                    return NULL;
                }
                options->in_file = optarg;
                break;

            case 'o':
                if (!optarg || optarg[0] == '\0') {
                    fprintf(stderr, "ERROR: --outfile requires a filename or '-' for stdout\n");
                    free(options);
                    return NULL;
                }
                options->out_file = optarg;
                break;

            case 'f':
                if (!optarg || optarg[0] == '\0') {
                    fprintf(stderr, "ERROR: --format requires a format (bin|xml|json|openstep)\n");
                    free(options);
                    return NULL;
                }
                if (!strncmp(optarg, "bin", 3)) {
                    options->out_fmt = PLIST_FORMAT_BINARY;
                } else if (!strncmp(optarg, "xml", 3)) {
                    options->out_fmt = PLIST_FORMAT_XML;
                } else if (!strncmp(optarg, "json", 4)) {
                    options->out_fmt = PLIST_FORMAT_JSON;
                } else if (!strncmp(optarg, "openstep", 8) ||
                           !strncmp(optarg, "ostep", 5)) {
                    options->out_fmt = PLIST_FORMAT_OSTEP;
                } else {
                    fprintf(stderr, "ERROR: Unsupported output format\n");
                    free(options);
                    return NULL;
                }
                break;

            case 'c':
                options->flags |= OPT_COMPACT;
                break;

            case 's':
                options->flags |= OPT_SORT;
                break;

            case 'p': {
                if (!optarg || optarg[0] == '\0') {
                    fprintf(stderr, "ERROR: --print requires a filename or '-' for stdin\n");
                    free(options);
                    return NULL;
                }
                options->in_file = optarg;
                options->out_fmt = PLIST_FORMAT_PRINT;

                char *env_fmt = getenv("PLIST_OUTPUT_FORMAT");
                if (env_fmt) {
                    if (!strcmp(env_fmt, "plutil")) {
                        options->out_fmt = PLIST_FORMAT_PLUTIL;
                    } else if (!strcmp(env_fmt, "limd")) {
                        options->out_fmt = PLIST_FORMAT_LIMD;
                    }
                }
                break;
            }

            case 'd':
                options->flags |= OPT_DEBUG;
                break;

            case 'h':
                free(options);
                return NULL;

            case 'v':
                printf("plistutil %s\n", libplist_version());
                exit(EXIT_SUCCESS);

            default:
                fprintf(stderr, "ERROR: Invalid option\n");
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
    size_t read_size = 0;
    size_t read_capacity = 4096;
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
            fprintf(stderr, "ERROR: Failed to allocate buffer to read from stdin\n");
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
        if(plist_entire == NULL)
        {
            fprintf(stderr, "ERROR: Failed to allocate buffer to read from file\n");
            free(options);
            return 1;
        }
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
        input_res = plist_from_memory(plist_entire, read_size, &root_node, NULL);
        if (input_res == PLIST_ERR_SUCCESS) {
            if (options->flags & OPT_SORT) {
                plist_sort(root_node);
            }
            if (options->out_fmt == PLIST_FORMAT_BINARY) {
                output_res = plist_to_bin(root_node, &plist_out, &size);
            } else if (options->out_fmt == PLIST_FORMAT_XML) {
                output_res = plist_to_xml(root_node, &plist_out, &size);
            } else if (options->out_fmt == PLIST_FORMAT_JSON) {
                output_res = plist_to_json(root_node, &plist_out, &size, !(options->flags & OPT_COMPACT));
            } else if (options->out_fmt == PLIST_FORMAT_OSTEP) {
                output_res = plist_to_openstep(root_node, &plist_out, &size, !(options->flags & OPT_COMPACT));
            } else {
                plist_write_to_stream(root_node, stdout, options->out_fmt, PLIST_OPT_PARTIAL_DATA);
                plist_free(root_node);
                free(plist_entire);
                free(options);
                return 0;
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
            case PLIST_ERR_CIRCULAR_REF:
                fprintf(stderr, "ERROR: Circular reference detected.\n");
                ret = 5;
                break;
            case PLIST_ERR_MAX_NESTING:
                fprintf(stderr, "ERROR: Output plist data exceeds maximum nesting depth.\n");
                ret = 4;
                break;
            case PLIST_ERR_FORMAT:
                fprintf(stderr, "ERROR: Input plist data is not compatible with output format.\n");
                ret = 2;
                break;
            default:
                fprintf(stderr, "ERROR: Failed to convert plist data (%d)\n", output_res);
                ret = 1;
                break;
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
            case PLIST_ERR_CIRCULAR_REF:
                fprintf(stderr, "ERROR: Circular reference detected in input plist data.\n");
                ret = 5;
                break;
            case PLIST_ERR_MAX_NESTING:
                fprintf(stderr, "ERROR: Input plist data exceeds maximum nesting depth.\n");
                ret = 4;
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
