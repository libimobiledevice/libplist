INCLUDES = -I$(top_srcdir)/include

BUILT_SOURCES = $(srcdir)/plist_wrap.c
SWIG_SOURCES = plist.i 

pkgpython_PYTHON = PList.py __init__.py
pkgpyexec_LTLIBRARIES = _PList.la
_PList_la_SOURCES = $(srcdir)/plist_wrap.c $(SWIG_SOURCES)
_PList_la_CFLAGS = $(PYTHON_CPPFLAGS) -I$(top_srcdir)/src
_PList_la_LDFLAGS = -module $(PYTHON_LDFLAGS)
_PList_la_LIBADD = ../src/libplist.la

$(srcdir)/plist_wrap.c : $(SWIG_SOURCES)
	$(SWIG) $(SWIG_PYTHON_OPT) $(INCLUDES) -I$(top_srcdir)/src -o $@ $<

