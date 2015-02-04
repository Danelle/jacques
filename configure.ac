dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(jacques, 0.1)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_PREFIX_DEFAULT(/usr)


# i18n
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=jacques
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_GLIB_GNU_GETTEXT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([ftruncate memset socket strchr strerror])


PKG_CHECK_MODULES(JACQUES, [glib-2.0 gio-2.0 gmodule-2.0 ])

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
		   src/jac/Makefile
           po/Makefile.in
		   ])
