AM_CFLAGS = $(GLOBAL_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/libcnary/include
AM_LDFLAGS =

noinst_PROGRAMS = plist_cmp plist_test

plist_cmp_SOURCES = plist_cmp.c
plist_cmp_LDADD = $(top_builddir)/src/libplist.la

plist_test_SOURCES = plist_test.c
plist_test_LDADD = $(top_builddir)/src/libplist.la

TESTS = \
	empty.test \
	small.test \
	medium.test \
	large.test \
	huge.test \
	bigarray.test \
	emptycmp.test \
	smallcmp.test \
	mediumcmp.test \
	largecmp.test \
	hugecmp.test \
	bigarraycmp.test

EXTRA_DIST = $(TESTS) data/1.plist data/2.plist data/3.plist data/4.plist data/5.plist data/6.plist

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) top_builddir=$(top_builddir)

clean-local:
	rm -f $(top_builddir)/test/data/*.out
