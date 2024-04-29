AM_CFLAGS = -Wall -Wextra -Werror
AM_CPPFLAGS = -include config.h

if WITH_CHECK
TESTS = unit_test leak_test
EXTRA_DIST = leak_test
check_PROGRAMS = unit_test

unit_test_SOURCES = unit_test.c \
    check_block.c \
    check_buffer.c \
    check_csv.c \
    check_json.c \
    check_delta.c \
    check_dict.c \
    check_head.c \
    check_list.c \
    check_table.c \
    check_utils.c \
    check_instance.c \
    check_patch.c
unit_test_CFLAGS = @CHECK_CFLAGS@
unit_test_LDADD = @CHECK_LIBS@ $(top_builddir)/lib/libleech.la
endif
