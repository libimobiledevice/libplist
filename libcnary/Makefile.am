AM_CFLAGS = $(GLOBAL_CFLAGS) -I$(top_srcdir)/libcnary/include
AM_LDFLAGS =

noinst_LTLIBRARIES = libcnary.la
libcnary_la_LIBADD = 
libcnary_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
libcnary_la_SOURCES = \
		       node.c \
		       list.c \
		       node_list.c \
		       iterator.c \
		       node_iterator.c \
		       include/node.h \
		       include/list.h \
		       include/node_list.h \
		       include/iterator.h \
		       include/node_iterator.h \
		       include/object.h
