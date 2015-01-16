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

bin_PROGRAMS = jacques \
	client

jacques_SOURCES = \
	main.c \
	config.h \
	config.c \
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
	struct.c

jacques_LDFLAGS = 

SUBLIBS = io/libjio.a  

jacques_LDADD = $(SUBLIBS) $(JACQUES_LIBS)

client_SOURCES =  \
	client.c

client_LDADD = $(SUBLIBS) $(JACQUES_LIBS)



SUBDIRS = \
	io \
	conf
