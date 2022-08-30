TESTS = check_leech check_utils
check_PROGRAMS = check_leech check_utils

check_leech_SOURCES = check_leech.c $(top_builddir)/src/leech.c
check_leech_CFLAGS = @CHECK_CFLAGS@
check_leech_LDADD = $(top_builddir)/src/libleech.la -lcheck_pic -lrt -lm -lsubunit @CHECK_LIBS@
check_leech_LDFLAGS = -pthread

check_utils_SOURCES = check_utils.c $(top_builddir)/src/utils.c
check_utils_CFLAGS = @CHECK_CFLAGS@
check_utils_LDADD = $(top_builddir)/src/libleech.la -lcheck_pic -lrt -lm -lsubunit @CHECK_LIBS@
check_utils_LDFLAGS = -pthread
