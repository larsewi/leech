
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
AC_ARG_WITH([check], AS_HELP_STRING([--with-check], [use check test framework]))
AC_ARG_WITH([psql-module], AS_HELP_STRING([--with-psql-module], [build PostgreSQL module]))
AC_ARG_WITH([csv-module], AS_HELP_STRING([--with-csv-module], [build CSV module]))

# Automake conditionals.
AM_CONDITIONAL([NDEBUG], [test "x$debug" = "xno"])
AM_CONDITIONAL([USE_CHECK], [test "x$with_check" = "xyes"])
AM_CONDITIONAL([BUILD_PSQL_MODULE], [test "x$with_psql_module" = "xyes"])
AM_CONDITIONAL([BUILD_CSV_MODULE], [test "x$with_csv_module" = "xyes"])

# Config constants.
AC_DEFINE([LCH_DEFAULT_MAX_CHAIN_LENGTH], 2048, [Default max chain length used in block garbage collector])
AC_DEFINE([LCH_JSON_PRETTY_INDENT_SIZE], 2, [Indent size used when composing pretty JSON])
AC_DEFINE([LCH_BUFFER_SIZE], 1024, [Initial buffer size allocated by leech])
AC_DEFINE([LCH_DICT_CAPACITY], 256, [Initial dictionary capacity allocated by leech])
AC_DEFINE([LCH_DICT_LOAD_FACTOR], 0.75f, [Initial dictionary capacity allocated by leech])
AC_DEFINE([LCH_LIST_CAPACITY], 256, [Initial list capacity allocated by leech])

# pkg-check modules.
AS_IF([test "x$with_check" = "xyes"], [PKG_CHECK_MODULES([CHECK], [check])])
AS_IF([test "x$with_psql_module" = "xyes"],
        [PKG_CHECK_MODULES([PSQL], [libpq],
                [AC_DEFINE([HAVE_LIBPQ], [1],
                        [Define to 1 if you have the <libpq-fe.h> header file.]
                )
        ])
])

AC_MSG_CHECKING([for debug option])
if test x"$debug" = x"yes"
then
    AC_MSG_RESULT(yes)
    CFLAGS="$CFLAGS -g3 -O0"
else
    AC_MSG_RESULT(no)
    CFLAGS="$CFLAGS -O2 -DNDEBUG"
fi

AC_CONFIG_FILES([Makefile lib/Makefile bin/Makefile tests/Makefile])
AC_OUTPUT
