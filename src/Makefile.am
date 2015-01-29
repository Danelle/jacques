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
	 -Ijac \
	 -Iio \
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
	utils.c \
	utils.h \
	log.c \
	log.h

jacques_LDFLAGS = 

SUBLIBS = \
	jac/libjac.a \
	io/libjio.a

jacques_LDADD = $(SUBLIBS) $(JACQUES_LIBS)

client_SOURCES =  \
	client.c

client_LDADD = $(SUBLIBS) $(JACQUES_LIBS)



SUBDIRS = \
	io \
	jac \
	modules

bin_SCRIPTS = flood.sh \
	batch.sh
