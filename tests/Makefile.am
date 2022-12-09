TESTS = unit_test

check_PROGRAMS = unit_test

unit_test_SOURCES = \
    main.c $(top_builddir)/leech/debug_messenger.c \
    check_buffer.c $(top_builddir)/leech/buffer.c \
    check_csv.c $(top_builddir)/leech/csv.c \
    check_dict.c $(top_builddir)/leech/dict.c \
    check_list.c $(top_builddir)/leech/list.c \
    check_utils.c $(top_builddir)/leech/utils.c

# unit_test_LDADD = -lcheck_pic -lrt -lm -lsubunit @CHECK_LIBS@
unit_test_LDADD = @CHECK_LIBS@

unit_test_LDFLAGS = -pthread
