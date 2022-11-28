AM_CPPFLAGS = -I$(top_builddir)/leech

noinst_PROGRAMS = simulate
simulate_LDADD = $(top_builddir)/leech/libleech.la $(top_builddir)/leech/libleech_csv.la -lcsv
simulate_SOURCES = simulate.c commands.c server.c
