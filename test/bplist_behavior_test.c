/*
 * bplist_behavior_test.c
 *
 * Tests binary plist parser behavior around the object data area:
 *
 * 1) Arrays and dictionaries are parsed correctly for different
 *    object reference sizes, including sizes that are not a power of 2.
 *
 * 2) Objects whose data extends beyond the object data area, i.e.
 *    into the offset table, must cause parsing to fail. This covers
 *    strings, extended size markers, array references and dictionary
 *    key/value references.
 *
 * The binary plists are assembled in memory with a minimal writer so
 * each test case only has to specify the object bytes.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "plist/plist.h"

#define MAX_BPLIST_SIZE 256

static void put_be64(uint8_t *p, uint64_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        p[i] = (uint8_t)(v & 0xFF);
        v >>= 8;
    }
}

/*
 * Assemble a binary plist from the given object bytes. offsets[] holds the
 * start of each object relative to the beginning of objs. A 1 byte offset
 * table is placed directly after the objects, followed by the trailer.
 */
static uint32_t build_bplist(uint8_t *out, const uint8_t *objs, size_t objs_len, const uint8_t *offsets, size_t num_objects, uint8_t ref_size, uint64_t root)
{
    size_t len = 0;
    size_t i;
    memcpy(out, "bplist00", 8);
    len = 8;
    memcpy(out + len, objs, objs_len);
    len += objs_len;
    size_t offset_table_offset = len;
    for (i = 0; i < num_objects; i++) {
        out[len++] = (uint8_t)(8 + offsets[i]);
    }
    memset(out + len, 0, 6);
    out[len + 6] = 1; /* offset size */
    out[len + 7] = ref_size;
    put_be64(out + len + 8, num_objects);
    put_be64(out + len + 16, root);
    put_be64(out + len + 24, offset_table_offset);
    len += 32;
    return (uint32_t)len;
}

static int check_int_item(plist_t item, uint64_t expected, const char *name)
{
    uint64_t val = 0;
    if (plist_get_node_type(item) != PLIST_INT) {
        fprintf(stderr, "%s: expected PLIST_INT item\n", name);
        return 0;
    }
    plist_get_uint_val(item, &val);
    if (val != expected) {
        fprintf(stderr, "%s: expected %" PRIu64 ", got %" PRIu64 "\n", name, expected, val);
        return 0;
    }
    return 1;
}

static int test_array_ref_sizes(void)
{
    /* [ 1, 2 ] with 1 byte object references */
    const uint8_t objs1[] = { 0xA2, 0x01, 0x02, 0x10, 0x01, 0x10, 0x02 };
    const uint8_t offs1[] = { 0, 3, 5 };
    /* [ 1, 2 ] with 3 byte object references */
    const uint8_t objs3[] = { 0xA2, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x10, 0x01, 0x10, 0x02 };
    const uint8_t offs3[] = { 0, 7, 9 };
    struct {
        const uint8_t *objs;
        size_t objs_len;
        const uint8_t *offs;
        uint8_t ref_size;
    } cases[] = {
        { objs1, sizeof(objs1), offs1, 1 },
        { objs3, sizeof(objs3), offs3, 3 },
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t buf[MAX_BPLIST_SIZE];
        uint32_t len = build_bplist(buf, cases[i].objs, cases[i].objs_len, cases[i].offs, 3, cases[i].ref_size, 0);
        plist_t root = NULL;
        plist_err_t err = plist_from_bin((const char*)buf, len, &root);
        if (err != PLIST_ERR_SUCCESS || !root) {
            fprintf(stderr, "array ref_size %u: plist_from_bin failed (err=%d)\n", cases[i].ref_size, err);
            plist_free(root);
            return 0;
        }
        if (plist_get_node_type(root) != PLIST_ARRAY || plist_array_get_size(root) != 2) {
            fprintf(stderr, "array ref_size %u: expected array with 2 items\n", cases[i].ref_size);
            plist_free(root);
            return 0;
        }
        if (!check_int_item(plist_array_get_item(root, 0), 1, "array item 0") ||
            !check_int_item(plist_array_get_item(root, 1), 2, "array item 1")) {
            plist_free(root);
            return 0;
        }
        plist_free(root);
    }
    return 1;
}

static int test_dict_ref_size(void)
{
    /* { "a": 5 } with 2 byte object references */
    const uint8_t objs[] = { 0xD1, 0x00, 0x01, 0x00, 0x02, 0x51, 'a', 0x10, 0x05 };
    const uint8_t offs[] = { 0, 5, 7 };
    uint8_t buf[MAX_BPLIST_SIZE];
    uint32_t len = build_bplist(buf, objs, sizeof(objs), offs, 3, 2, 0);
    plist_t root = NULL;
    plist_err_t err = plist_from_bin((const char*)buf, len, &root);
    if (err != PLIST_ERR_SUCCESS || !root) {
        fprintf(stderr, "dict ref_size 2: plist_from_bin failed (err=%d)\n", err);
        plist_free(root);
        return 0;
    }
    if (plist_get_node_type(root) != PLIST_DICT || plist_dict_get_size(root) != 1) {
        fprintf(stderr, "dict ref_size 2: expected dict with 1 entry\n");
        plist_free(root);
        return 0;
    }
    if (!check_int_item(plist_dict_get_item(root, "a"), 5, "dict value")) {
        plist_free(root);
        return 0;
    }
    plist_free(root);
    return 1;
}

static int expect_rejected(const uint8_t *objs, size_t objs_len, const uint8_t *offs, size_t num_objects, uint64_t root_index, const char *name)
{
    uint8_t buf[MAX_BPLIST_SIZE];
    uint32_t len = build_bplist(buf, objs, objs_len, offs, num_objects, 1, root_index);
    plist_t root = NULL;
    plist_err_t err = plist_from_bin((const char*)buf, len, &root);
    if (err == PLIST_ERR_SUCCESS || root != NULL) {
        fprintf(stderr, "%s: expected failure, got err=%d root=%p\n", name, err, (void*)root);
        plist_free(root);
        return 0;
    }
    return 1;
}

static int test_data_beyond_object_area_is_rejected(void)
{
    int ok = 1;

    /* string claims 5 bytes, only 2 are present before the offset table */
    const uint8_t str_objs[] = { 0x55, 'a', 'b' };
    const uint8_t str_offs[] = { 0 };
    ok &= expect_rejected(str_objs, sizeof(str_objs), str_offs, 1, 0, "truncated string");

    /* extended size marker, but no size integer before the offset table */
    const uint8_t ext_objs[] = { 0x5F };
    const uint8_t ext_offs[] = { 0 };
    ok &= expect_rejected(ext_objs, sizeof(ext_objs), ext_offs, 1, 0, "missing size integer");

    /* array claims 3 references, only 2 are present before the offset table */
    const uint8_t arr_objs[] = { 0x10, 0x01, 0x10, 0x02, 0xA3, 0x00, 0x01 };
    const uint8_t arr_offs[] = { 0, 2, 4 };
    ok &= expect_rejected(arr_objs, sizeof(arr_objs), arr_offs, 3, 2, "truncated array references");

    /* dict has its key reference, but the value reference is missing */
    const uint8_t dict_objs[] = { 0x51, 'a', 0x10, 0x05, 0xD1, 0x00 };
    const uint8_t dict_offs[] = { 0, 2, 4 };
    ok &= expect_rejected(dict_objs, sizeof(dict_objs), dict_offs, 3, 2, "truncated dict references");

    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_array_ref_sizes();
    ok &= test_dict_ref_size();
    ok &= test_data_beyond_object_area_is_rejected();

    return ok ? 0 : 1;
}
