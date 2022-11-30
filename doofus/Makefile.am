AM_CPPFLAGS = -I$(top_builddir)/leech

noinst_PROGRAMS = doofus
doofus_LDADD = \
    $(top_builddir)/leech/libleech.la \
    $(top_builddir)/leech/libleech_csv.la \
    $(top_builddir)/leech/libleech_psql.la
doofus_SOURCES = doofus.c
