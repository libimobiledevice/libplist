 /*
 * plist_write_fuzzer.cc
 * plist writer fuzz target for libFuzzer
 *
 * Copyright (c) 2017 Nikias Bassen All Rights Reserved.
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

#include <plist/plist.h>

#include <stddef.h>
#include <stdint.h>

static plist_format_t select_output_format(uint8_t mode)
{
    switch (mode % 6) {
    case 0: return PLIST_FORMAT_XML;
    case 1: return PLIST_FORMAT_JSON;
    case 2: return PLIST_FORMAT_OSTEP;
    case 3: return PLIST_FORMAT_PRINT;
    case 4: return PLIST_FORMAT_LIMD;
    default: return PLIST_FORMAT_PLUTIL;
    }
}

static plist_write_options_t select_options(uint8_t opt)
{
    unsigned int options = PLIST_OPT_NONE;

    if (opt & 0x01) {
        options |= PLIST_OPT_COMPACT;
    }
    if (opt & 0x02) {
        options |= PLIST_OPT_COERCE;
    }
    if (opt & 0x04) {
        options |= PLIST_OPT_NO_NEWLINE;
    }
    if (opt & 0x08) {
        options |= PLIST_OPT_PARTIAL_DATA;
    }
    if (opt & 0x10) {
        options |= PLIST_OPT_INDENT;
        options |= PLIST_OPT_INDENT_BY(1);
    }

    return static_cast<plist_write_options_t>(options);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (!data || size < 3 || size > 4096) {
        return 0;
    }

    const uint8_t mode = data[size - 2];
    const uint8_t opt = data[size - 1];

    plist_t root = NULL;
    plist_format_t in_format = PLIST_FORMAT_NONE;

    plist_err_t err = plist_from_memory(
        reinterpret_cast<const char*>(data),
        static_cast<uint32_t>(size - 2),
        &root,
        &in_format
    );

    if (err != PLIST_ERR_SUCCESS || !root) {
        plist_free(root);
        return 0;
    }

    char* output = NULL;
    uint32_t output_len = 0;

    (void)plist_write_to_string(
        root,
        &output,
        &output_len,
        select_output_format(mode),
        select_options(opt)
    );

    plist_mem_free(output);
    plist_free(root);

    return 0;
}
