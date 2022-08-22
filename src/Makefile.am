if SIMULATE
    noinst_PROGRAMS = simulate
    simulate_SOURCES = server.c common.c
    simulate_LDADD = libleech.la libleech_csv.la
endif

lib_LTLIBRARIES = libleech.la libleech_csv.la
libleech_la_SOURCES = leech.c
libleech_csv_la_SOURCES = leech_csv.c
