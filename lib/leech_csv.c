#include "leech_csv.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

LCH_List *LCH_TableReadCallbackCSV(const char *const locator) {
  assert(locator != NULL);

  FILE *file = fopen(locator, "r");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for reading: %s", locator,
                  strerror(errno));
    return NULL;
  }

  size_t size;
  if (!LCH_FileSize(file, &size)) {
    LCH_LOG_ERROR("Failed to get size of file '%s'", locator);
    fclose(file);
    return NULL;
  }

  char buffer[size];
  if (fread((void *)buffer, 1, size, file) != size) {
    LCH_LOG_ERROR("Failed to read file '%s': %s", locator, strerror(errno));
    fclose(file);
    return NULL;
  }
  fclose(file);

  LCH_List *table = LCH_ParseCSV(buffer);
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV file '%s'", locator, strerror(errno));
    return NULL;
  }

  return table;
}

bool LCH_TableWriteCallbackCSV(const char *const locator,
                               const LCH_List *const table) {
  assert(locator != NULL);
  assert(table != NULL);

  LCH_Buffer *const buffer = LCH_ComposeCSV(table);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to compose CSV for file '%s'", locator);
    return false;
  }

  char *const csv = LCH_BufferGet(buffer);
  if (csv == NULL) {
    LCH_LOG_ERROR(
        "Failed to get string from buffer after composing CSV for file '%s'",
        locator);
    return false;
  }

  const size_t length = LCH_BufferLength(buffer);
  LCH_BufferDestroy(buffer);

  FILE *const file = fopen(locator, "w");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for writing: %s", locator,
                  strerror(errno));
    return false;
  }

  if (fwrite(csv, 1, length, file) != length) {
    LCH_LOG_ERROR("Failed to write composed CSV (%zu bytes) to file '%s': %s",
                  length, locator, strerror(errno));
    fclose(file);
    return false;
  }

  fclose(file);
  free(csv);
  return true;
}
