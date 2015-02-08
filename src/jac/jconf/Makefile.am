## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(JACQUES_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libjconf.la


libjconf_la_SOURCES = \
	jconf.c \
	struct.c \
	private.h

libjconf_la_LDFLAGS = 

libjconf_la_LIBADD = $(JACQUES_LIBS)

