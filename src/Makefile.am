## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
	$(JACQUES_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -pthread \
	 -Ijac \
	 -Iio \
	 -Ijconf \
	 -g

bin_PROGRAMS = jacques \
	client

jacques_SOURCES = \
	main.c \
	config.h \
	config.c \
	master.h \
	master.c \
	server.c \
	server.h \
	worker.c \
	worker.h \
	utils.c \
	utils.h \
	log.c \
	log.h

jacques_LDFLAGS =

SUBLIBS = \
	jac/libjac.a \
	io/libjio.a \
	jconf/libjconf.a

jacques_LDADD = $(SUBLIBS) $(JACQUES_LIBS)

client_SOURCES =  \
	client.c

client_LDADD = $(SUBLIBS) $(JACQUES_LIBS)


nobase_include_HEADERS = \
	jac/mod.h \
	jac/hooks.h \
	jac/struct.h \
	jac/jac.h \
	jconf/jconf.h \
	jconf/struct.h


SUBDIRS = \
	io \
	jac \
	jconf \
	modules

bin_SCRIPTS = flood.sh \
	batch.sh
