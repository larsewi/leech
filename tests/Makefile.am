TESTS = unit_test leak_test acceptance_test

AM_CPPFLAGS = -include config.h

pkglib_LTLIBRARIES = module.la
module_la_SOURCES = module.c
module_la_LDFLAGS = -avoid-version -module -shared

check_PROGRAMS = unit_test
unit_test_SOURCES = \
    unit_test.c \
    check_block.c \
    check_buffer.c \
    check_csv.c \
    check_json.c \
    check_delta.c \
    check_dict.c \
    check_head.c \
    check_leech_csv.c \
    check_list.c \
    check_table.c \
    check_utils.c

unit_test_CFLAGS = \
    @CHECK_CFLAGS@

unit_test_LDADD = \
    @CHECK_LIBS@ \
    $(top_builddir)/lib/libleech.la \
    $(top_builddir)/lib/libleech_csv.la \
    $(top_builddir)/lib/libleech_psql.la
