include_HEADERS = \
        $(top_builddir)/leech/leech.h \
        $(top_builddir)/leech/leech_csv.h \
        $(top_builddir)/leech/leech_psql.h

lib_LTLIBRARIES = \
        libleech.la \
        libleech_csv.la \
        libleech_psql.la

libleech_la_SOURCES = \
        debug_messenger.c \
        instance.c \
        utils.c \
        list.c \
        dict.c \
        buffer.c \
        table.c

libleech_csv_la_SOURCES = \
        leech_csv.c \
        debug_messenger.c \
        list.c

libleech_psql_la_SOURCES = \
        leech_psql.c \
        debug_messenger.c \
        list.c
