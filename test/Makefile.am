AM_CFLAGS = $(GLOBAL_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/libcnary/include
AM_LDFLAGS =

noinst_PROGRAMS = plist_cmp plist_test

plist_cmp_SOURCES = plist_cmp.c
plist_cmp_LDADD = ../src/libplist.la

plist_test_SOURCES = plist_test.c
plist_test_LDADD = ../src/libplist.la

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

