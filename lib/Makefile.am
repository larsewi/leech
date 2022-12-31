include_HEADERS = \
        $(top_builddir)/lib/leech.h \
        $(top_builddir)/lib/leech_csv.h \
        $(top_builddir)/lib/leech_psql.h

lib_LTLIBRARIES = \
        libleech.la \
        libleech_csv.la \
        libleech_psql.la

libleech_la_SOURCES = \
        block.h block.c \
        buffer.h buffer.c \
        csv.h csv.c \
        debug_messenger.h debug_messenger.c \
        dict.h dict.c \
        instance.h instance.c \
        list.h list.c \
        table.h table.c \
        utils.h utils.c

libleech_la_LIBADD = @OPENSSL_LIBS@

libleech_csv_la_SOURCES = \
        buffer.c \
        leech_csv.c \
        debug_messenger.c \
        list.c

libleech_psql_la_SOURCES = \
        leech_psql.c \
        debug_messenger.c \
        list.c
