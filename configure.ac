
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([leech], [0.1.0], [https://github.com/larsewi/leech/issues], [leech],
        [https://github.com/larsewi/leech])
AC_CONFIG_SRCDIR([lib/leech.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_AR

LT_INIT([dlopen])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h unistd.h stdint.h fcntl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset strchr strspn strdup strerror strndup mkdir rmdir])

# Configure arguments.
AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug], [enable debugging]),
              [debug=$enableval], [debug=no])

# Automake conditionals.
AM_CONDITIONAL([NDEBUG], [test x"$debug" = x"no"])

# pkg-check modules.
PKG_CHECK_MODULES([CHECK], [check])
PKG_CHECK_MODULES([PSQL], [libpq])

AC_CONFIG_FILES([Makefile lib/Makefile bin/Makefile tests/Makefile])
AC_OUTPUT
