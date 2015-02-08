## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -g

noinst_LIBRARIES = libjio.a

libjio_a_SOURCES =  \
	jio.h \
	jsocket.c \
	jsocket.h \
	pack.c \
	pack.h \
	jpoll.h \
	jpoll.c


libjio_a_CPPFLAGS =  $(JACQUES_CFLAGS)
