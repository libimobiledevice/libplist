#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#include <string.h>
#include <plist/plist.h>

void print_plist(plist_t pl)
{
	char *xml = NULL;
	uint32_t xlen = 0;
	plist_to_xml(pl, &xml, &xlen);
	if (xml) {
		printf("%s\n", xml);
	}
	free(xml);
}

int main(int argc, char** argv)
{
	int err = 0;
	char *xml = NULL;
	uint32_t xlen = 0;
	plist_t iii = plist_new_int(0);

	/* test 1 */
	plist_set_uint_val(iii, 0x8000000000000000LL);
	plist_to_xml(iii, &xml, &xlen);
	const char* match1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<integer>9223372036854775808</integer>\n"
		"</plist>\n";
	if (strcmp(xml, match1) != 0) {
		printf("ERROR: plist_set_uint_val with 0x8000000000000000LL failed\n");
		err++;
	} else {
		printf("SUCCESS: plist_set_uint_val with 0x8000000000000000LL\n");
	}
	free(xml);
	xml = NULL;

	/* test 2 */
	plist_set_int_val(iii, 0x8000000000000000LL);
	plist_to_xml(iii, &xml, &xlen);
	const char* match2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<integer>-9223372036854775808</integer>\n"
		"</plist>\n";
	if (strcmp(xml, match2) != 0) {
		printf("ERROR: plist_set_int_val with 0x8000000000000000LL failed\n");
		err++;
	} else {
		printf("SUCCESS: plist_set_int_val with 0x8000000000000000LL\n");
	}
	free(xml);
	xml = NULL;

	/* test 3 */
	plist_set_uint_val(iii, (uint64_t)-1LL);
	plist_to_xml(iii, &xml, &xlen);
	const char* match3 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<integer>18446744073709551615</integer>\n"
		"</plist>\n";
	if (strcmp(xml, match3) != 0) {
		printf("ERROR: plist_set_uint_val with (uint64_t)-1LL failed\n");
		err++;
	} else {
		printf("SUCCESS: plist_set_uint_val with (uint64_t)-1LL\n");
	}
	free(xml);
	xml = NULL;

	/* test 4 */
	plist_set_int_val(iii, -1LL);
	plist_to_xml(iii, &xml, &xlen);
	const char* match4 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<integer>-1</integer>\n"
		"</plist>\n";
	if (strcmp(xml, match4) != 0) {
		printf("ERROR: plist_set_int_val with -1LL failed\n");
		err++;
	} else {
		printf("SUCCESS: plist_set_int_val with -1LL\n");
	}
	free(xml);
	xml = NULL;

	/* test 5 */
	plist_set_uint_val(iii, 0x8000000000000001LL);
	plist_to_xml(iii, &xml, &xlen);
	const char* match5 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<integer>9223372036854775809</integer>\n"
		"</plist>\n";
	if (strcmp(xml, match5) != 0) {
		printf("ERROR: plist_set_uint_val with 0x8000000000000001LL failed\n");
		err++;
	} else {
		printf("SUCCESS: plist_set_uint_val with 0x8000000000000001LL\n");
	}
	free(xml);
	xml = NULL;

	/* test 6 */
	plist_set_uint_val(iii, 18446744073709551615uLL);
	plist_to_xml(iii, &xml, &xlen);
	const char* match6 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<integer>18446744073709551615</integer>\n"
		"</plist>\n";
	if (strcmp(xml, match6) != 0) {
		printf("ERROR: plist_set_uint_val with 0x8000000000000001LL failed\n");
		err++;
	} else {
		printf("SUCCESS: plist_set_uint_val with 0x8000000000000001LL\n");
	}
	free(xml);
	xml = NULL;

	return (err > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
