## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -g

noinst_LIBRARIES = libjconf.a


libjconf_a_SOURCES = \
	jconf.c \
	struct.c \
	private.h

libjconf_a_CPPFLAGS = $(JACQUES_CFLAGS)
