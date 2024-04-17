AM_CFLAGS = -Wall -Wextra -Werror
AM_CPPFLAGS = -include config.h

noinst_PROGRAMS = leech

leech_SOURCES = main.c \
    common.c common.h \
    commit.c commit.h \
    diff.c diff.h \
    rebase.c rebase.h \
    patch.c patch.h
leech_LDADD = @PSQL_LIBS@ $(top_builddir)/lib/libleech.la
leech_CFLAGS = @PSQL_CFLAGS@
