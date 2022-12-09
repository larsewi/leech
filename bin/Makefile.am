AM_CPPFLAGS = -I$(top_srcdir)/lib

noinst_PROGRAMS = leech
leech_SOURCES = leech.c
leech_LDADD = \
    $(top_builddir)/lib/libleech.la \
    $(top_builddir)/lib/libleech_csv.la \
    $(top_builddir)/lib/libleech_psql.la
