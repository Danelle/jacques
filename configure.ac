dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(jacques, 0.1)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC


LT_INIT

PKG_CHECK_MODULES(JACQUES, [glib-2.0 gmodule-2.0 ])

AC_SEARCH_LIBS([pthread_create],[pthread],
			   [],
			   AC_MSG_ERROR([POSIX thread library required]))
AC_CHECK_FUNC(epoll_create,
			  [],
			 AC_MSG_ERROR([epoll not found]))


AC_OUTPUT([
		   Makefile
		   src/Makefile
		   src/io/Makefile
src/conf/Makefile
		   ])
