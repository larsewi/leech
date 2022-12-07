AM_CPPFLAGS = -I$(top_srcdir)/leech

noinst_PROGRAMS = doofus
doofus_SOURCES = doofus.c
doofus_LDADD = \
    $(top_builddir)/leech/libleech.la \
    $(top_builddir)/leech/libleech_csv.la \
    $(top_builddir)/leech/libleech_psql.la
