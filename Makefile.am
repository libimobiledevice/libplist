AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = libcnary src include tools docs

if HAVE_CYTHON
SUBDIRS += cython
endif

if BUILD_TESTS
SUBDIRS += test
endif

if BUILD_FUZZERS
SUBDIRS += fuzz
endif

EXTRA_DIST = \
	README.md \
	git-version-gen

dist-hook:
	@if ! git diff --quiet; then echo "Uncommitted changes present; not releasing"; exit 1; fi
	echo $(VERSION) > $(distdir)/.tarball-version

docs/html: $(top_builddir)/doxygen.cfg $(top_srcdir)/include/plist/*.h
	rm -rf docs/html
	doxygen doxygen.cfg

docs/README.doxygen.md: README.md
	awk '/\#\# Table of Contents/{while(getline && $$0 != ""){}{print "[TOC]"}}1' README.md > $@

docs: doxygen.cfg docs/README.doxygen.md docs/html
