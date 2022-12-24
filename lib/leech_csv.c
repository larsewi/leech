#include "leech_csv.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "buffer.h"
#include "csv.h"
#include "utils.h"

LCH_List *LCH_TableReadCallbackCSV(const void *const locator) {
  assert(locator != NULL);

  const char *const path = (const char *)locator;

  FILE *file = fopen(path, "r");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for reading: %s", path,
                  strerror(errno));
    return NULL;
  }

  size_t size;
  if (!LCH_FileSize(file, &size)) {
    LCH_LOG_ERROR("Failed to get size of file '%s'", path);
    fclose(file);
    return NULL;
  }

  char buffer[size];
  if (fread((void *)buffer, 1, size, file) != size) {
    LCH_LOG_ERROR("Failed to read file '%s': %s", path, strerror(errno));
    fclose(file);
    return NULL;
  }
  fclose(file);

  LCH_List *table = LCH_CSVParse(buffer);
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV file '%s'", path, strerror(errno));
    return NULL;
  }

  return table;
}

bool LCH_TableWriteCallbackCSV(const void *const locator,
                               const LCH_List *const table) {
  assert(locator != NULL);
  assert(table != NULL);

  const char *const path = (const char *)locator;

  LCH_Buffer *const buffer = LCH_CSVCompose(table);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to compose CSV for file '%s'", path);
    return false;
  }

  char *const csv = LCH_BufferGet(buffer);
  if (csv == NULL) {
    LCH_LOG_ERROR(
        "Failed to get string from buffer after composing CSV for file '%s'",
        path);
    return false;
  }

  const size_t length = LCH_BufferLength(buffer);
  LCH_BufferDestroy(buffer);

  FILE *const file = fopen(path, "w");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for writing: %s", path,
                  strerror(errno));
    return false;
  }

  if (fwrite(csv, 1, length, file) != length) {
    LCH_LOG_ERROR("Failed to write composed CSV (%zu bytes) to file '%s': %s",
                  length, path, strerror(errno));
    fclose(file);
    return false;
  }

  fclose(file);
  free(csv);
  return true;
}
