#include <assert.h>
#include <errno.h>
#include <string.h>

#include "leech.h"

typedef struct LCH_Table {
  LCH_List *records;
  const void *writeLocator;
  bool (*writeCallback)(const void *, const LCH_List *);
} LCH_Table;

LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->primaryFields != NULL);
  assert(createInfo->subsidiaryFields != NULL);
  assert(createInfo->readLocator != NULL);
  assert(createInfo->writeLocator != NULL);
  assert(createInfo->readCallback != NULL);
  assert(createInfo->writeCallback != NULL);

  LCH_Table *table = (LCH_Table *)malloc(sizeof(LCH_Table));
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for table: %s", strerror(errno));
    return NULL;
  }

  table->records = createInfo->readCallback(createInfo->readLocator);
  if (table->records == NULL) {
    LCH_LOG_ERROR("Failed to read table data");
    return NULL;
  }

  table->writeLocator = createInfo->writeLocator;
  table->writeCallback = createInfo->writeCallback;

  return table;
}

void LCH_TableDestroy(LCH_Table *table) {
  if (table == NULL) {
    return;
  }
  LCH_ListDestroy(table->records);
  free(table);
}
