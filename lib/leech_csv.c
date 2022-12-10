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
    return NULL;
  }

  char buffer[size];

  return NULL;
}

bool LCH_TableWriteCallbackCSV(const char *const locator,
                               const LCH_List *const table) {
  return false;
}
