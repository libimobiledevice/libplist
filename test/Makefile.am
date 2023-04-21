AM_CFLAGS = \
	$(GLOBAL_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/libcnary/include
AM_CPPFLAGS = $(AM_CFLAGS)

AM_LDFLAGS =

noinst_PROGRAMS = \
	plist_cmp \
	plist_test \
	plist_test++ \
	integer_set_test \
	plist_btest \
	plist_jtest \
	plist_otest

plist_cmp_SOURCES = plist_cmp.c
plist_cmp_LDADD = \
	$(top_builddir)/src/libplist-2.0.la \
	$(top_builddir)/libcnary/libcnary.la

plist_test___SOURCES = plist_test++.cpp
plist_test___LDADD = $(top_builddir)/src/libplist++-2.0.la

plist_test_SOURCES = plist_test.c
plist_test_LDADD = $(top_builddir)/src/libplist-2.0.la

integer_set_test_SOURCES = integer_set.c
integer_set_test_LDADD = $(top_builddir)/src/libplist-2.0.la

plist_btest_SOURCES = plist_btest.c
plist_btest_LDADD = $(top_builddir)/src/libplist-2.0.la

plist_jtest_SOURCES = plist_jtest.c
plist_jtest_LDADD = $(top_builddir)/src/libplist-2.0.la

plist_otest_SOURCES = plist_otest.c
plist_otest_LDADD = $(top_builddir)/src/libplist-2.0.la

TESTS = \
	empty.test \
	small.test \
	medium.test \
	large.test \
	huge.test \
	bigarray.test \
	empty++.test \
	small++.test \
	medium++.test \
	large++.test \
	huge++.test \
	bigarray++.test \
	dates.test \
	timezone1.test \
	timezone2.test \
	signedunsigned1.test \
	signedunsigned2.test \
	signedunsigned3.test \
	hex.test \
	order.test \
	recursion.test \
	entities.test \
	empty_keys.test \
	amp.test \
	invalid_tag.test \
	cdata.test \
	offsetsize.test \
	refsize.test \
	malformed_dict.test \
	uid.test \
	integer_set.test \
	json1.test \
	json2.test \
	json3.test \
	json-invalid-types.test \
	json-int64-min-max.test \
	ostep1.test \
	ostep2.test \
	ostep-strings.test \
	ostep-comments.test \
	ostep-invalid-types.test

EXTRA_DIST = \
	$(TESTS) \
	data/1.plist \
	data/2.plist \
	data/3.plist \
	data/4.plist \
	data/5.plist \
	data/6.plist \
	data/7.plist \
	data/amp.plist \
	data/cdata.plist \
	data/dictref1byte.bplist \
	data/dictref2bytes.bplist \
	data/dictref3bytes.bplist \
	data/dictref4bytes.bplist \
	data/dictref5bytes.bplist \
	data/dictref6bytes.bplist \
	data/dictref7bytes.bplist \
	data/dictref8bytes.bplist \
	data/empty_keys.plist \
	data/entities.plist \
	data/hex.plist \
	data/invalid_tag.plist \
	data/malformed_dict.bplist \
	data/off1byte.bplist \
	data/off2bytes.bplist \
	data/off3bytes.bplist \
	data/off4bytes.bplist \
	data/off5bytes.bplist \
	data/off6bytes.bplist \
	data/off7bytes.bplist \
	data/off8bytes.bplist \
	data/offxml.plist \
	data/order.bplist \
	data/order.plist \
	data/recursion.bplist \
	data/signed.bplist \
	data/signed.plist \
	data/signedunsigned.bplist \
	data/signedunsigned.plist \
	data/unsigned.bplist \
	data/unsigned.plist \
	data/uid.bplist \
	data/data.bplist \
	data/j1.json \
	data/j2.json \
	data/int64_min_max.json \
	data/o1.ostep \
	data/o2.ostep \
	data/o3.ostep \
	data/test.strings

TESTS_ENVIRONMENT = \
	top_srcdir=$(top_srcdir) \
	top_builddir=$(top_builddir)

clean-local:
	if test -d $(top_builddir)/test/data; then cd $(top_builddir)/test/data && rm -f *.out *.bin *.xml; fi
