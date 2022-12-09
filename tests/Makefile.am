TESTS = unit_test

check_PROGRAMS = unit_test

unit_test_SOURCES = \
    main.c $(top_builddir)/lib/debug_messenger.c \
    check_buffer.c $(top_builddir)/lib/buffer.c \
    check_csv.c $(top_builddir)/lib/csv.c \
    check_dict.c $(top_builddir)/lib/dict.c \
    check_list.c $(top_builddir)/lib/list.c \
    check_utils.c $(top_builddir)/lib/utils.c

# unit_test_LDADD = -lcheck_pic -lrt -lm -lsubunit @CHECK_LIBS@
unit_test_LDADD = @CHECK_LIBS@

unit_test_LDFLAGS = -pthread
