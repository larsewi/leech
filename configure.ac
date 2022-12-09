AC_INIT([leech],[0.1.0])

AC_CONFIG_SRCDIR([lib/leech.h])

AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AM_PATH_CHECK

AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug], [enable debugging]),
              [debug=$enableval],
              [debug=no])
AM_CONDITIONAL([NDEBUG], [test x"$debug" = x"no"])

AC_PROG_CC
AM_PROG_AR

LT_INIT

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile lib/Makefile bin/Makefile tests/Makefile])

AC_OUTPUT
