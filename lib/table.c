#include "table.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "csv.h"
#include "definitions.h"
#include "leech.h"
#include "list.h"
#include "utils.h"

typedef struct LCH_Table {
  const char *identifier;
  const char *primary_fields;
  const char *subsidiary_fields;
  const void *read_locator;
  const void *write_locator;
  LCH_List *(*read_callback)(const void *);
  bool (*write_callback)(const void *, const LCH_List *);
  bool (*insert_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*delete_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*update_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
} LCH_Table;

const char *LCH_TableGetIdentifier(const LCH_Table *const self) {
  assert(self != NULL);
  return self->identifier;
}

LCH_Table *LCH_TableCreate(const LCH_TableCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->identifier != NULL);
  assert(createInfo->primary_fields != NULL);
  assert(createInfo->subsidiary_fields != NULL);
  assert(createInfo->read_locator != NULL);
  assert(createInfo->write_locator != NULL);
  assert(createInfo->read_callback != NULL);
  assert(createInfo->write_callback != NULL);
  assert(createInfo->insert_callback != NULL);
  assert(createInfo->delete_callback != NULL);
  assert(createInfo->update_callback != NULL);

  LCH_Table *table = (LCH_Table *)calloc(1, sizeof(LCH_Table));
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  table->identifier = createInfo->identifier;
  table->primary_fields = createInfo->primary_fields;
  table->subsidiary_fields = createInfo->subsidiary_fields;
  table->read_locator = createInfo->read_locator;
  table->write_locator = createInfo->write_locator;
  table->read_callback = createInfo->read_callback;
  table->write_callback = createInfo->write_callback;
  table->insert_callback = createInfo->insert_callback;
  table->delete_callback = createInfo->delete_callback;
  table->update_callback = createInfo->update_callback;

  return table;
}

LCH_Dict *LCH_TableLoadNewState(const LCH_Table *const table) {
  LCH_List *const records = table->read_callback(table->read_locator);
  if (records == NULL) {
    return NULL;
  }

  LCH_Dict *const data =
      LCH_TableToDict(records, table->primary_fields, table->subsidiary_fields);
  if (data == NULL) {
    LCH_ListDestroy(records);
    return NULL;
  }

  LCH_ListDestroy(records);

  return data;
}

static LCH_Dict *LoadSnapshot(const char *const path) {
  LCH_List *table = LCH_CSVParseFile(path);
  if (table == NULL) {
    return NULL;
  }

  LCH_Dict *snapshot = LCH_DictCreate();
  if (snapshot == NULL) {
    return NULL;
  }

  assert(LCH_ListLength(table) % 2 == 0);
  char *key = NULL, *val = NULL;
  for (size_t i = 0; i < LCH_ListLength(table); i++) {
    const LCH_List *const record = LCH_ListGet(table, i);

    LCH_Buffer *const buffer = LCH_CSVComposeRecord(record);
    if (buffer == NULL) {
      LCH_ListDestroy(table);
      LCH_DictDestroy(snapshot);
      return NULL;
    }

    char *str = LCH_BufferStringDup(buffer);
    if (str == NULL) {
      LCH_LOG_ERROR("Failed to get snapshot string from string buffer");
      free(key);
      free(val);
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(table);
      LCH_DictDestroy(snapshot);
      return NULL;
    }
    LCH_BufferDestroy(buffer);

    if (i % 2 == 0) {
      key = str;
      continue;
    }
    val = str;

    if (!LCH_DictSet(snapshot, key, val, free)) {
      free(key);
      free(val);
      LCH_ListDestroy(table);
      LCH_DictDestroy(snapshot);
      return NULL;
    }
    free(key);
    key = NULL;
    val = NULL;
  }

  LCH_ListDestroy(table);
  return snapshot;
}

LCH_Dict *LCH_TableLoadOldState(const LCH_Table *const table,
                                const char *const work_dir) {
  assert(table != NULL);
  assert(work_dir != NULL);
  assert(table->identifier != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                    table->identifier)) {
    return NULL;
  }

  LCH_Dict *const snapshot =
      (LCH_IsRegularFile(path)) ? LoadSnapshot(path) : LCH_DictCreate();
  if (snapshot == NULL) {
    LCH_LOG_ERROR("Failed to load snapshot for table %s", table->identifier);
    return NULL;
  }

  return snapshot;
}

void LCH_TableDestroy(LCH_Table *table) { free(table); }
