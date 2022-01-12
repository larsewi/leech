AC_INIT(leech, 0.1.0)
AC_CONFIG_SRCDIR([src/leech.c])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([foreign])

AC_ARG_ENABLE([simulate],
[  --enable-simulate       build binaries for simulation],
[case "${enableval}" in
  yes) simulate=true ;;
  no)  simulate=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-simulate]) ;;
esac],[simulate=false])
AM_CONDITIONAL([SIMULATE], [test x$simulate = xtrue])

AC_PROG_CC
LT_INIT

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT