/*
 * xml_behavior_test.c
 *
 * Tests XML parser behavior for correctness and specification compliance:
 *
 * 1) A <plist> element must contain exactly one root value node.
 *    Any additional value nodes after the first root object must
 *    cause parsing to fail.
 *
 * 2) Dictionaries of the form:
 *        <dict>
 *          <key>CF$UID</key>
 *          <integer>...</integer>
 *        </dict>
 *    must be converted to PLIST_UID nodes during XML parsing,
 *    including when they appear nested inside other containers.
 *
 * These tests ensure proper root handling and UID node conversion
 * when parsing XML property lists.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "plist/plist.h"

static int test_nested_cfuid_converts_to_uid(void)
{
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
        "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
        "<plist version=\"1.0\">"
        "  <dict>"
        "    <key>obj</key>"
        "    <dict>"
        "      <key>CF$UID</key>"
        "      <integer>7</integer>"
        "    </dict>"
        "  </dict>"
        "</plist>";

    plist_t root = NULL;
    plist_err_t err = plist_from_xml(xml, (uint32_t)strlen(xml), &root);
    if (err != PLIST_ERR_SUCCESS || !root) {
        fprintf(stderr, "nested CF$UID: plist_from_xml failed (err=%d)\n", err);
        plist_free(root);
        return 0;
    }

    if (plist_get_node_type(root) != PLIST_DICT) {
        fprintf(stderr, "nested CF$UID: root is not dict\n");
        plist_free(root);
        return 0;
    }

    plist_t obj = plist_dict_get_item(root, "obj");
    if (!obj) {
        fprintf(stderr, "nested CF$UID: missing key 'obj'\n");
        plist_free(root);
        return 0;
    }

    if (plist_get_node_type(obj) != PLIST_UID) {
        fprintf(stderr, "nested CF$UID: expected PLIST_UID, got %d\n",
                plist_get_node_type(obj));
        plist_free(root);
        return 0;
    }

    uint64_t uid = 0;
    plist_get_uid_val(obj, &uid);
    if (uid != 7) {
        fprintf(stderr, "nested CF$UID: expected uid=7, got %" PRIu64 "\n", uid);
        plist_free(root);
        return 0;
    }

    plist_free(root);
    return 1;
}

static int test_extra_root_value_is_rejected(void)
{
    /* Two root values inside <plist> must be rejected */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<plist version=\"1.0\">"
        "  <string>one</string>"
        "  <string>two</string>"
        "</plist>";

    plist_t root = NULL;
    plist_err_t err = plist_from_xml(xml, (uint32_t)strlen(xml), &root);

    /* Must fail, and root must be NULL (consistent with other parsers) */
    if (err == PLIST_ERR_SUCCESS || root != NULL) {
        fprintf(stderr, "extra root value: expected failure, got err=%d root=%p\n",
                err, (void*)root);
        plist_free(root);
        return 0;
    }
    return 1;
}

static int test_scalar_then_extra_node_is_rejected(void)
{
    /* Scalar root followed by another node must be rejected */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<plist version=\"1.0\">"
        "  <true/>"
        "  <dict><key>A</key><string>x</string></dict>"
        "</plist>";

    plist_t root = NULL;
    plist_err_t err = plist_from_xml(xml, (uint32_t)strlen(xml), &root);

    if (err == PLIST_ERR_SUCCESS || root != NULL) {
        fprintf(stderr, "scalar then extra node: expected failure, got err=%d root=%p\n",
                err, (void*)root);
        plist_free(root);
        return 0;
    }
    return 1;
}

static int test_scalar_with_comment_is_ok(void)
{
    /* Comment after the single root value is not an extra value node */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<plist version=\"1.0\">"
        "  <string>ok</string>"
        "  <!-- trailing comment -->"
        "</plist>";

    plist_t root = NULL;
    plist_err_t err = plist_from_xml(xml, (uint32_t)strlen(xml), &root);
    if (err != PLIST_ERR_SUCCESS || !root) {
        fprintf(stderr, "scalar + comment: expected success, got err=%d\n", err);
        plist_free(root);
        return 0;
    }
    if (plist_get_node_type(root) != PLIST_STRING) {
        fprintf(stderr, "scalar + comment: expected root string, got %d\n",
                plist_get_node_type(root));
        plist_free(root);
        return 0;
    }
    plist_free(root);
    return 1;
}

static int test_negative_zero_real_is_preserved(void)
{
    plist_t root = plist_new_real(-0.0);
    char *xml = NULL;
    uint32_t len = 0;
    plist_t parsed = NULL;
    double value = 0.0;
    int ok = 1;

    if (plist_to_xml(root, &xml, &len) != PLIST_ERR_SUCCESS || !xml) {
        fprintf(stderr, "negative zero: plist_to_xml failed\n");
        plist_free(root);
        return 0;
    }
    if (!strstr(xml, "-0.0")) {
        fprintf(stderr, "negative zero: missing -0.0 in XML output\n");
        ok = 0;
    }
    if (plist_from_xml(xml, len, &parsed) != PLIST_ERR_SUCCESS || !parsed) {
        fprintf(stderr, "negative zero: plist_from_xml failed\n");
        ok = 0;
    } else if (plist_get_node_type(parsed) != PLIST_REAL) {
        fprintf(stderr, "negative zero: expected real node\n");
        ok = 0;
    } else {
        plist_get_real_val(parsed, &value);
        if (!signbit(value)) {
            fprintf(stderr, "negative zero: sign was lost\n");
            ok = 0;
        }
    }

    free(xml);
    plist_free(parsed);
    plist_free(root);
    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_nested_cfuid_converts_to_uid();
    ok &= test_extra_root_value_is_rejected();
    ok &= test_scalar_then_extra_node_is_rejected();
    ok &= test_scalar_with_comment_is_ok();
    ok &= test_negative_zero_real_is_preserved();

    return ok ? 0 : 1;
}
