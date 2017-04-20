AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = libcnary src include tools test
if HAVE_CYTHON
SUBDIRS += cython
endif

docs/html: $(top_builddir)/doxygen.cfg $(top_srcdir)/include/plist/*.h
	rm -rf docs
	doxygen doxygen.cfg

docs: doxygen.cfg docs/html

clean-local:
	rm -rf docs
