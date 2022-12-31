
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([leech],[0.1.0],[https://github.com/larsewi/leech/issues],
        [leech],[https://github.com/larsewi/leech])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR

# Checks for libraries.
## FIXME: Replace 'main' with a function in '-lcheck_pic':
## AC_CHECK_LIB([check_pic], [main])
## FIXME: Replace 'main' with a function in '-lcrypto':
## AC_CHECK_LIB([crypto], [main])
## FIXME: Replace 'main' with a function in '-lm':
## AC_CHECK_LIB([m], [main])
## FIXME: Replace 'main' with a function in '-lrt':
## AC_CHECK_LIB([rt], [main])
## FIXME: Replace 'main' with a function in '-lssl':
## AC_CHECK_LIB([ssl], [main])
## FIXME: Replace 'main' with a function in '-lsubunit':
## AC_CHECK_LIB([subunit], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset strchr strdup strerror strndup])

# Configure arguments.
AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug], [enable debugging]),
              [debug=$enableval],
              [debug=no])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_PATH_CHECK
AM_CONDITIONAL([NDEBUG], [test x"$debug" = x"no"])

LT_INIT

AC_CONFIG_FILES([Makefile lib/Makefile bin/Makefile tests/Makefile])
AC_OUTPUT
