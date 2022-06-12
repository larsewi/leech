if SIMULATE
    noinst_PROGRAMS = node
endif

node = node.c
node_LDADD = libleech.la libleech_csv.la

lib_LTLIBRARIES = libleech.la libleech_csv.la
libleech_la_SOURCES = leech.c
libleech_csv_la_SOURCES = leech_csv.c
