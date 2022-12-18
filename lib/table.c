#include <assert.h>
#include <errno.h>
#include <string.h>

#include "leech.h"

typedef struct LCH_Table {
  LCH_List *primaryFields;
  LCH_List *subsidiaryFields;
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

  LCH_Table *table = (LCH_Table *)calloc(1, sizeof(LCH_Table));
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for table: %s", strerror(errno));
    return NULL;
  }

  LCH_List *primary = LCH_ParseCSV(createInfo->primaryFields);
  LCH_ListSort(primary, (int (*)(const void *, const void *))strcmp);
  table->primaryFields = primary;

  LCH_List *subsidiary = LCH_ParseCSV(createInfo->subsidiaryFields);
  LCH_ListSort(subsidiary, (int (*)(const void *, const void *))strcmp);
  table->subsidiaryFields = subsidiary;

  LCH_List *records = createInfo->readCallback(createInfo->readLocator);
  if (records == NULL) {
    LCH_LOG_ERROR("Failed to read table");
    LCH_ListDestroy(subsidiary);
    LCH_ListDestroy(primary);
    free(table);
  }

  LCH_List *header = (LCH_List *)LCH_ListGet(records, 0);

  LCH_ListDestroy(records);

  table->writeLocator = createInfo->writeLocator;
  table->writeCallback = createInfo->writeCallback;

  return table;
}

void LCH_TableDestroy(LCH_Table *table) {
  if (table == NULL) {
    return;
  }
  LCH_ListDestroy(table->primaryFields);
  LCH_ListDestroy(table->subsidiaryFields);
  LCH_ListDestroy(table->records);
  free(table);
}
