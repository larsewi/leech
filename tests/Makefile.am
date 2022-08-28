TESTS = check_leech_utils
check_PROGRAMS = check_leech_utils
check_leech_utils_SOURCES = check_leech_utils.c $(top_builddir)/src/leech_utils.c
check_leech_utils_CFLAGS = @CHECK_CFLAGS@
check_leech_utils_LDADD = $(top_builddir)/src/libleech_utils.la -lcheck_pic -lrt -lm -lsubunit @CHECK_LIBS@
check_leech_utils_LDFLAGS = -pthread
