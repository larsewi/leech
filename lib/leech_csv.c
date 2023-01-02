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
  return LCH_CSVParseFile(path);
}

bool LCH_TableWriteCallbackCSV(const void *const locator,
                               const LCH_List *const table) {
  assert(locator != NULL);
  assert(table != NULL);
  const char *const path = (const char *)locator;
  return LCH_CSVComposeFile(table, path);
}
