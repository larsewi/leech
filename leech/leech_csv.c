#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <csv.h>

#include "leech_csv.h"
#include "utils.h"
#include "debug_messenger.h"


static struct table {
  bool success;
  LCH_List *record;
  LCH_List *records;
};


static void field_callback(char *str, size_t len, struct table *table) {
  assert(str != NULL);
  assert(table != NULL);
  assert(table->records != NULL);

  if (table->record == NULL) {
    table->record = LCH_ListCreate();
    if (table->record == NULL) {
      return;
    }
  }

  char *field = strdup(str);
  if (field == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return;
  }

  table->success = LCH_ListAppend(table->record, (void *)field, free);
}

static void record_callback(int ch, struct table *table) {
  assert(table != NULL);
  assert(table->records != NULL);
  assert(table->record != NULL);

  table->success |= LCH_ListAppend(table->records, (void *)table->record, LCH_ListDestroy);
  table->record = NULL;
}


LCH_List *LCH_TableReadCallbackCSV(const char *const locator) {
  struct csv_parser parser;
  int ret = csv_init(&parser, CSV_APPEND_NULL);
  if (ret != 0) {
    LCH_LOG_ERROR("Failed to initialize CSV parser");
    return NULL;
  }

  FILE *file = fopen(locator, "rb");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for reading: %s", locator, strerror(errno));
    return NULL;
  }
  size_t bytes_read;

  char buffer[LCH_BUFFER_SIZE];
  buffer[0] = '\0';

  struct table table = {
    .record = NULL,
    .records = LCH_ListCreate(),
  };

  do {
    table.success = false;
    bytes_read = fread(buffer, 1, sizeof(buffer), file);
    const size_t bytes_parsed = csv_parse(&parser, buffer, bytes_read, field_callback, record_callback, &table);
    if (bytes_parsed != bytes_read) {
      LCH_LOG_ERROR("Failed to parse CSV file '%s': '%s'", locator, csv_strerror(csv_error(&parser)));
      csv_free(&parser);
      fclose(file);
      free(table.records);
      return NULL;
    }
  } while (bytes_read > 0 && table.success);

  csv_fini(&parser, field_callback, record_callback, &table);
  csv_free(&parser);
  fclose(file);

  if (!table.success) {
    LCH_LOG_ERROR("Failed to parse CSV file '%s'", locator);
    free(table.records);
    return NULL;
  }

  return table.records;
}

bool LCH_TableWriteCallbackCSV(const char *const locator, const LCH_List *const table) {
  FILE *file = fopen(file, "r");
  if (!LCH_FileWriteTable(file, table)) {
    return false;
  }
  fclose(file);
  return true;
}
