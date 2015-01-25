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
	 -Iconf \
	 -g

bin_PROGRAMS = jacques \
	client

jacques_SOURCES = \
	main.c \
	config.h \
	core.c \
	core.h \
	server.c \
	server.h \
	worker.c \
	worker.h \
	mod.c \
	mod.h \
	mod-pub.h \
	struct.h \
	struct.c \
	hooks.c \
	hooks.h \
	utils.c \
	utils.h

jacques_LDFLAGS = 

SUBLIBS = \
	conf/libjconf.a \
	io/libjio.a

jacques_LDADD = $(SUBLIBS) $(JACQUES_LIBS)

client_SOURCES =  \
	client.c

client_LDADD = $(SUBLIBS) $(JACQUES_LIBS)



SUBDIRS = \
	io \
	conf
