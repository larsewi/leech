#include "leech_psql.h"

#include <assert.h>

LCH_List *LCH_TableReadCallbackPSQL(const char *const locator) {
  assert(locator != NULL);
  return NULL;
}

bool LCH_TableWriteCallbackPSQL(const char *const locator,
                                const LCH_List *const table) {
  assert(locator != NULL);
  assert(table != NULL);
  return false;
}
