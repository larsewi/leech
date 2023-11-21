AM_CFLAGS = -Wall -Wextra -Werror
AM_CPPFLAGS = -include config.h

noinst_PROGRAMS = leech

leech_SOURCES = leech.c \
    common.c common.h \
    commit.c commit.h \
    diff.c diff.h \
    rebase.c rebase.h \
    patch.c patch.h

leech_LDADD = \
    $(top_builddir)/lib/libleech.la \
    $(top_builddir)/lib/libleech_csv.la \
    $(top_builddir)/lib/libleech_psql.la
