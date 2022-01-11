if SIMULATE
    noinst_PROGRAMS = collector emitter
endif

collector_SOURCES = collector.c
collector_LDADD = libleech.la

emitter_SOURCES = emitter.c
emitter_LDADD = libleech.la

lib_LTLIBRARIES = libleech.la
libleech_la_SOURCES = leech.c
