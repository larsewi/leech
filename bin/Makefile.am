AM_CFLAGS = -Wall -Wextra -Werror
AM_CPPFLAGS = -include config.h

noinst_PROGRAMS =

if BUILD_TEST_BINARY
noinst_PROGRAMS += leech

leech_SOURCES = main.c \
    common.c common.h \
    commit.c commit.h \
    diff.c diff.h \
    rebase.c rebase.h \
    patch.c patch.h \
    history.c history.h \
    purge.c purge.h
leech_LDADD = @PSQL_LIBS@ $(top_builddir)/lib/libleech.la
leech_CFLAGS = @PSQL_CFLAGS@
endif
