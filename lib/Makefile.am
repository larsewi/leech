include_HEADERS = \
        $(top_builddir)/lib/leech.h \
        $(top_builddir)/lib/leech_csv.h \
        $(top_builddir)/lib/leech_psql.h

lib_LTLIBRARIES = \
        libleech.la \
        libleech_csv.la \
        libleech_psql.la

libleech_la_SOURCES = \
        buffer.c \
        csv.c \
        debug_messenger.c \
        dict.c \
        instance.c \
        list.c \
        table.c \
        utils.c

libleech_csv_la_SOURCES = \
        buffer.c \
        leech_csv.c \
        debug_messenger.c \
        list.c

libleech_psql_la_SOURCES = \
        leech_psql.c \
        debug_messenger.c \
        list.c
