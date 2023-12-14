AM_CFLAGS = -Wall -Wextra -Werror
AM_CPPFLAGS = -include config.h

include_HEADERS = \
        $(top_builddir)/lib/leech.h \
        $(top_builddir)/lib/leech_csv.h \
        $(top_builddir)/lib/leech_psql.h

lib_LTLIBRARIES = \
        libleech.la \
        libleech_csv.la \
        libleech_psql.la

libleech_la_SOURCES = leech.c \
        block.h block.c \
        buffer.h buffer.c \
        csv.h csv.c \
        json.h json.c \
        debug_messenger.h debug_messenger.c \
        dict.h dict.c \
        delta.h delta.c \
        head.h head.c \
        instance.h instance.c \
        list.h list.c \
        table.h table.c \
        utils.h utils.c \
        sha1.h sha1.c

libleech_csv_la_SOURCES = \
        buffer.c \
        leech_csv.c \
        debug_messenger.c \
        list.c

libleech_psql_la_SOURCES = \
        leech_psql.c \
        debug_messenger.c \
        list.c
