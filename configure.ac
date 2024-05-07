
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([leech], [0.1.19], [https://github.com/larsewi/leech/issues], [leech],
        [https://github.com/larsewi/leech])
AC_CONFIG_SRCDIR([lib/leech.h])

AC_DEFINE([LCH_DEFAULT_PREFERED_CHAIN_LENGTH], 2048,
          [Default prefered chain length used when puring blocks])
AC_DEFINE([LCH_JSON_PRETTY_INDENT_SIZE], 2,
          [Indent size used when composing pretty JSON])
AC_DEFINE([LCH_BUFFER_SIZE], 1024,
          [Initial buffer size allocated by leech])
AC_DEFINE([LCH_LIST_CAPACITY], 256,
          [Initial list capacity allocated by leech])
AC_DEFINE([LCH_DICT_CAPACITY], 256,
          [Initial dictionary capacity allocated by leech])
AC_DEFINE([LCH_DICT_LOAD_FACTOR], 0.75f,
          [Initial dictionary capacity allocated by leech])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_AR
AC_PROG_AWK
AC_PROG_LN_S
AC_PROG_MAKE_SET

LT_INIT([dlopen win32-dll])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for header files.
AC_CHECK_HEADER_STDBOOL
AC_CHECK_HEADERS([arpa/inet.h unistd.h stdint.h fcntl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_MODE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for libraries.
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([m], [floor])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset strchr strspn strdup strerror strpbrk strtol strndup mkdir rmdir])

# Enable debugging
AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug], [enable debugging]),
              [debug=$enableval], [debug=no])
AM_CONDITIONAL([NDEBUG], [test "x$debug" = "xno"])
AC_MSG_CHECKING([for debug option])
if test x"$debug" = x"yes"
then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
    CFLAGS="$CFLAGS -DNDEBUG"
fi

# Compile with check test framework
AC_ARG_WITH([check-framework], AS_HELP_STRING([--with-check-framework], [use check framework]))
AM_CONDITIONAL([WITH_CHECK], [test "x$with_check_framework" = "xyes"])
AS_IF([test "x$with_check_framework" = "xyes"], [PKG_CHECK_MODULES([CHECK], [check])])

# Compile test binary
AC_ARG_WITH([test-binary], AS_HELP_STRING([--with-test-binary], [compile test binary (not intended for production)]))
AM_CONDITIONAL([BUILD_TEST_BINARY], [test "x$with_test_binary" = "xyes"])

# Compile CSV module
AC_ARG_WITH([csv-module], AS_HELP_STRING([--with-csv-module],
            [build CSV module (not intended for production)]))
AM_CONDITIONAL([BUILD_CSV_MODULE], [test "x$with_csv_module" = "xyes"])

# Compile PostgreSQL module
AC_ARG_WITH([psql-module], AS_HELP_STRING([--with-psql-module],
            [build PostgreSQL module (not intended for production)]))
AM_CONDITIONAL([BUILD_PSQL_MODULE], [test "x$with_psql_module" = "xyes"])
AS_IF([test "x$with_psql_module" = "xyes"],
        [PKG_CHECK_MODULES([PSQL], [libpq],
                [AC_DEFINE([HAVE_LIBPQ], [1],
                        [Define to 1 if you have the <libpq-fe.h> header file.]
                )
        ])
])

AC_CONFIG_FILES([Makefile lib/Makefile bin/Makefile tests/Makefile])
AC_OUTPUT
