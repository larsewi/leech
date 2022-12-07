AC_INIT([leech],[0.1.0])

AC_CONFIG_SRCDIR([leech/leech.c])

AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AM_PATH_CHECK

# PKG_CHECK_MODULES([CHECK], [check >= 0.15.2])

AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug], [enable debugging]),
              [debug=$enableval],
              [debug=no])
AM_CONDITIONAL([NDEBUG], [test x"$debug" = x"no"])

AM_PROG_AR
AC_PROG_CC

LT_INIT

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile leech/Makefile doofus/Makefile tests/Makefile])

AC_OUTPUT
