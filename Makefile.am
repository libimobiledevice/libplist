AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = libcnary src include tools test docs

if HAVE_CYTHON
SUBDIRS += cython
endif

if BUILD_FUZZERS
SUBDIRS += fuzz
endif

EXTRA_DIST = \
	README.md \
	git-version-gen

dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

docs/html: $(top_builddir)/doxygen.cfg $(top_srcdir)/include/plist/*.h
	rm -rf docs/html
	doxygen doxygen.cfg

docs: doxygen.cfg docs/html
