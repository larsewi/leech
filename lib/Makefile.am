AM_CFLAGS = -Wall -Wextra -Werror
AM_CPPFLAGS = -include config.h

include_HEADERS = $(top_builddir)/lib/leech.h
lib_LTLIBRARIES = libleech.la
pkglib_LTLIBRARIES = leech_csv.la leech_psql.la

leech_csv_la_SOURCES = leech_csv.c
leech_csv_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

leech_psql_la_SOURCES = leech_psql.c
leech_psql_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
leech_psql_la_CFLAGS = @PSQL_CFLAGS@

libleech_la_SOURCES = leech.c \
        block.h block.c \
        buffer.h buffer.c \
        csv.h csv.c \
        json.h json.c \
        logger.h logger.c \
        dict.h dict.c \
        delta.h delta.c \
        patch.h patch.c \
        head.h head.c \
        instance.h instance.c \
        list.h list.c \
        table.h table.c \
        utils.h utils.c \
        sha1.h sha1.c
libleech_la_LDFLAGS = -ldl
