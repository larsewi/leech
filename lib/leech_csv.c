#include "leech_csv.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "buffer.h"
#include "csv.h"
#include "utils.h"

LCH_List *LCH_TableReadCallbackCSV(const void *const locator) {
  assert(locator != NULL);
  const char *const filename = locator;
  return LCH_CSVParseFile(filename);
}

bool LCH_TableWriteCallbackCSV(const void *const locator,
                               const LCH_List *const table) {
  assert(locator != NULL);
  assert(table != NULL);
  const char *const filename = locator;
  return LCH_CSVComposeFile(table, filename);
}

bool LCH_TableInsertCallbackCSV(const void *const locator,
                                const LCH_List *const inserts) {
  const char *const filename = locator;
  LCH_List *const table = LCH_CSVParseFile(filename);
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV file '%s'.", filename);
    return false;
  }

  (void) inserts;

  return true;
}
