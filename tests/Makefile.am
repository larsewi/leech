AM_CFLAGS = -Wall -Wextra -Werror
AM_CPPFLAGS = -include config.h

if WITH_CHECK
TESTS = unit_test leak_test
EXTRA_DIST = leak_test
check_PROGRAMS = unit_test

unit_test_SOURCES = unit_test.c \
    unit/check_block.c \
    unit/check_buffer.c \
    unit/check_csv.c \
    unit/check_json.c \
    unit/check_delta.c \
    unit/check_dict.c \
    unit/check_head.c \
    unit/check_list.c \
    unit/check_table.c \
    unit/check_utils.c \
    unit/check_instance.c \
    unit/check_patch.c
unit_test_CFLAGS = @CHECK_CFLAGS@
unit_test_LDADD = @CHECK_LIBS@ $(top_builddir)/lib/libleech.la
endif
