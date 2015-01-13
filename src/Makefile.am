## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(JACQUES_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -pthread \
	 -g

bin_PROGRAMS = jacques

jacques_SOURCES = \
	main.c \
	config.h \
	config.c \
	core.c \
	core.h \
	server.c \
	server.h \
	worker.c \
	worker.h 

jacques_LDFLAGS = 

SUBLIBS = jio/libjio.a 

jacques_LDADD = $(SUBLIBS) $(JACQUES_LIBS)


SUBDIRS = \
	jio
