AC_INIT([leech],[0.1.0])
AC_CONFIG_SRCDIR([src/leech.c])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
PKG_CHECK_MODULES([CHECK], [check >= 0.9.6])

AM_PROG_AR
AC_PROG_CC
LT_INIT

AC_CONFIG_FILES([Makefile src/Makefile simulate/Makefile tests/Makefile])
AC_OUTPUT
