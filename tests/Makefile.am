TESTS = unit_test

check_PROGRAMS = unit_test

unit_test_SOURCES = main.c check_buffer.c check_csv.c check_dict.c \
    check_list.c check_utils.c

unit_test_LDADD = -lcheck_pic -lrt -lm -lsubunit @CHECK_LIBS@ \
    $(top_builddir)/lib/libleech.la \
    $(top_builddir)/lib/libleech_csv.la \
    $(top_builddir)/lib/libleech_psql.la
# unit_test_LDADD = @CHECK_LIBS@


unit_test_LDFLAGS = -pthread
