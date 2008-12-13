INCLUDES = -I$(top_srcdir)/include

AM_CFLAGS = $(libxml2_CFLAGS) $(libglib2_CFLAGS) 
AM_LDFLAGS = $(libxml2_LIBS) $(libglib2_LIBS)

lib_LTLIBRARIES = libplist.la
libplist_la_SOURCES = plist.c bplist.c xplist.c utils.c
