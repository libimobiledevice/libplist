AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src include plutil swig

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libplist-1.0.pc

doc:
	doxygen doxygen.cfg

indent:
	indent -kr -ut -ts4 -l120 src/*.c src/*.h plutil/*.c
