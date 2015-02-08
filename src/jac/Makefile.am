## Process this file with automake to produce Makefile.in


lib_LIBRARIES = libjac.a

libjac_a_SOURCES =  \
	jconfstruct.c \
	jconf.c \
	mod.c \
	hooks.c \
	struct.c 


libjac_a_CPPFLAGS =  $(JACQUES_CFLAGS)

libjac_a_LIBADD = jconf/libjconf.la

SUBDIRS=jconf
