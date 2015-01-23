AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = libcnary src include tools $(CYTHON_SUB) test

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libplist.pc libplist++.pc

docs/html: $(top_builddir)/doxygen.cfg $(top_srcdir)/include/plist/*.h
	rm -rf docs
	doxygen doxygen.cfg

docs: doxygen.cfg docs/html

clean-local:
	rm -rf docs
