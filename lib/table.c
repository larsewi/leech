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

static char *AddUniqueIdToRecord(const char *const record, const char *const id,
                                 size_t **const pos) {
  LCH_List *const list = LCH_CSVParseRecord(record);
  if (list == NULL) {
    return NULL;
  }

  char *const id_dup = strdup(id);
  if (id_dup == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    LCH_ListDestroy(list);
    return NULL;
  }

  if (!LCH_ListAppend(list, id_dup, free)) {
    free(id_dup);
    LCH_ListDestroy(list);
    return NULL;
  }

  if (*pos == NULL) { /* find position */
    *pos = malloc(sizeof(size_t));
    if (*pos == NULL) {
      LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
      LCH_ListDestroy(list);
      return NULL;
    }

    LCH_ListSort(list, (int (*)(const void *, const void *))strcmp);

    **pos =
        LCH_ListIndex(list, id, (int (*)(const void *, const void *))strcmp);
  } else { /* use position */
    const size_t len = LCH_ListLength(list);
    LCH_ListSwap(list, **pos, len - 1);
    for (size_t i = len - 2; i > **pos; i--) {
      LCH_ListSwap(list, i, i + 1);
    }
  }

  LCH_Buffer *const composed = LCH_CSVComposeRecord(list);
  LCH_ListDestroy(list);
  if (composed == NULL) {
    return NULL;
  }

  return LCH_BufferToString(composed);
}

static LCH_Dict *AddUniqueIdToDict(const LCH_Dict *const dict,
                                   const char *const unqiue_id,
                                   size_t **const pos) {
  LCH_Dict *new_dict = LCH_DictCreate();
  if (new_dict == NULL) {
    return NULL;
  }

  LCH_List *const keys = LCH_DictGetKeys(dict);
  if (keys == NULL) {
    LCH_DictDestroy(new_dict);
    return NULL;
  }

  for (size_t i = 0; i < LCH_ListLength(keys); i++) {
    const char *const old_key = LCH_ListGet(keys, i);
    assert(old_key != NULL);

    char *const value = LCH_DictGet(dict, old_key);

    char *const new_key = AddUniqueIdToRecord(old_key, unqiue_id, pos);
    if (new_key == NULL) {
      LCH_ListDestroy(keys);
      LCH_DictDestroy(new_dict);
      return NULL;
    }

    if (!LCH_DictSet(new_dict, new_key, value, NULL)) {
      free(new_key);
      LCH_ListDestroy(keys);
      LCH_DictDestroy(new_dict);
      return NULL;
    }

    free(new_key);
  }

  LCH_ListDestroy(keys);
  return new_dict;
}

bool LCH_TablePatch(const LCH_Table *const table, const LCH_Delta *const patch,
                    const char *const uid_field, const char *const uid_value) {
  assert(table != NULL);
  assert(patch != NULL);
  assert(uid_field != NULL);
  assert(uid_value != NULL);

  const char *const path = table->write_locator;

  size_t *pos = NULL;
  char *const primary_fields =
      AddUniqueIdToRecord(table->primary_fields, uid_field, &pos);
  if (primary_fields == NULL) {
    LCH_LOG_ERROR(
        "Failed to add unique identifier '%s' to primary field header for "
        "table '%s'.",
        uid_field, table->identifier);
    return false;
  }

  const LCH_Dict *const inserts = LCH_DeltaGetInsertions(patch);
  assert(inserts != NULL);
  LCH_Dict *const new_inserts = AddUniqueIdToDict(inserts, uid_value, &pos);
  if (new_inserts == NULL) {
    LCH_LOG_ERROR(
        "Failed to add unique identifier '%s' to inserts for table '%s'.",
        uid_value, table->identifier);
    free(pos);
    free(primary_fields);
    return false;
  }
  if (!table->insert_callback(path, primary_fields, table->subsidiary_fields,
                              new_inserts)) {
    LCH_LOG_ERROR("Failed to patch insertions for table '%s'.",
                  table->identifier);
    LCH_DictDestroy(new_inserts);
    free(pos);
    free(primary_fields);
    return false;
  }
  LCH_DictDestroy(new_inserts);

  const LCH_Dict *const deletions = LCH_DeltaGetDeletions(patch);
  assert(deletions != NULL);
  LCH_Dict *const new_deletions = AddUniqueIdToDict(deletions, uid_value, &pos);
  if (new_deletions == NULL) {
    LCH_LOG_ERROR(
        "Failed to add unique identifier '%s' to deletions for table '%s'.",
        uid_value, table->identifier);
    free(pos);
    free(primary_fields);
    return false;
  }
  if (!table->delete_callback(path, primary_fields, table->subsidiary_fields,
                              new_deletions)) {
    LCH_LOG_ERROR("Failed to patch deletions for table '%s'.",
                  table->identifier);
    free(primary_fields);
    free(pos);
    LCH_DictDestroy(new_deletions);
    return false;
  }
  LCH_DictDestroy(new_deletions);

  const LCH_Dict *const modifications = LCH_DeltaGetModifications(patch);
  assert(modifications != NULL);
  LCH_Dict *const new_modifications =
      AddUniqueIdToDict(modifications, uid_value, &pos);
  if (new_modifications == NULL) {
    LCH_LOG_ERROR(
        "Failed to add unique identifier '%s' to modifications for table '%s'.",
        uid_value, table->identifier);
    free(primary_fields);
    free(pos);
    return false;
  }
  if (!table->update_callback(path, primary_fields, table->subsidiary_fields,
                              new_modifications)) {
    LCH_LOG_ERROR("Failed to patch modifications for table '%s'.",
                  table->identifier);
    free(primary_fields);
    free(pos);
    LCH_DictDestroy(new_modifications);
    return false;
  }
  free(primary_fields);
  free(pos);
  LCH_DictDestroy(new_modifications);

  return true;
}

void LCH_TableDestroy(LCH_Table *table) { free(table); }
