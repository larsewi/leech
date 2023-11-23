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

typedef struct LCH_TableDefinition {
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
} LCH_TableDefinition;

const char *LCH_TableDefinitionGetIdentifier(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->identifier;
}

const char *LCH_TableDefinitionGetPrimaryFields(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->primary_fields;
}

const char *LCH_TableDefinitionGetSubsidiaryFields(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->subsidiary_fields;
}

LCH_TableDefinition *LCH_TableDefinitionCreate(
    const LCH_TableDefinitionCreateInfo *const create_info) {
  assert(create_info != NULL);
  assert(create_info->identifier != NULL);
  assert(create_info->primary_fields != NULL);
  assert(create_info->read_locator != NULL);
  assert(create_info->write_locator != NULL);
  assert(create_info->read_callback != NULL);
  assert(create_info->write_callback != NULL);
  assert(create_info->insert_callback != NULL);
  assert(create_info->delete_callback != NULL);
  assert(create_info->update_callback != NULL);

  LCH_TableDefinition *definition =
      (LCH_TableDefinition *)calloc(1, sizeof(LCH_TableDefinition));
  if (definition == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  definition->identifier = create_info->identifier;
  definition->primary_fields = create_info->primary_fields;
  definition->subsidiary_fields = create_info->subsidiary_fields;
  definition->read_locator = create_info->read_locator;
  definition->write_locator = create_info->write_locator;
  definition->read_callback = create_info->read_callback;
  definition->write_callback = create_info->write_callback;
  definition->insert_callback = create_info->insert_callback;
  definition->delete_callback = create_info->delete_callback;
  definition->update_callback = create_info->update_callback;

  return definition;
}

LCH_Dict *LCH_TableDefinitionLoadNewState(
    const LCH_TableDefinition *const self) {
  LCH_List *const table = self->read_callback(self->read_locator);
  if (table == NULL) {
    return NULL;
  }

  LCH_Dict *const state = LCH_TableToDict(table, self->primary_fields,
                                          self->subsidiary_fields, true);
  if (state == NULL) {
    LCH_ListDestroy(table);
    return NULL;
  }

  LCH_ListDestroy(table);

  return state;
}

LCH_Dict *LCH_TableDefinitionLoadOldState(const LCH_TableDefinition *const self,
                                          const char *const work_dir) {
  assert(self != NULL);
  assert(work_dir != NULL);
  assert(self->identifier != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                    self->identifier)) {
    return NULL;
  }

  if (LCH_IsRegularFile(path)) {
    LCH_List *const table = LCH_CSVParseFile(path);
    if (table == NULL) {
      return NULL;
    }

    LCH_Dict *const state = LCH_TableToDict(table, self->primary_fields,
                                            self->subsidiary_fields, true);
    if (state == NULL) {
      LCH_ListDestroy(table);
      return NULL;
    }

    return state;
  }

  LCH_Dict *const snapshot = LCH_DictCreate();
  return snapshot;
}

bool LCH_TableStoreNewState(const LCH_TableDefinition *const self,
                            const char *const work_dir,
                            const LCH_Dict *const state) {
  LCH_List *const table = LCH_DictToTable(state, self->primary_fields,
                                          self->subsidiary_fields, true);
  if (table == NULL) {
    return NULL;
  }

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                    self->identifier)) {
    return false;
  }

  const bool success = LCH_CSVComposeFile(table, path);
  LCH_ListDestroy(table);
  return success;
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
    *pos = (size_t *)malloc(sizeof(size_t));
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

  LCH_Buffer *composed = NULL;
  if (!LCH_CSVComposeRecord(&composed, list)) {
    LCH_ListDestroy(list);
    return NULL;
  }
  LCH_ListDestroy(list);

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
    const char *const old_key = (char *)LCH_ListGet(keys, i);
    assert(old_key != NULL);

    char *const value = (char *)LCH_DictGet(dict, old_key);

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

bool LCH_TableDefinitionPatch(const LCH_TableDefinition *const self,
                              const LCH_Delta *const patch,
                              const char *const uid_field,
                              const char *const uid_value) {
  assert(self != NULL);
  assert(patch != NULL);
  assert(uid_field != NULL);
  assert(uid_value != NULL);

  const char *const path = (char *)self->write_locator;

  size_t *pos = NULL;
  char *const primary_fields =
      AddUniqueIdToRecord(self->primary_fields, uid_field, &pos);
  if (primary_fields == NULL) {
    LCH_LOG_ERROR(
        "Failed to add unique identifier '%s' to primary field header for "
        "table '%s'.",
        uid_field, self->identifier);
    return false;
  }

  const LCH_Dict *const inserts = LCH_DeltaGetInsertions(patch);
  assert(inserts != NULL);
  LCH_Dict *const new_inserts = AddUniqueIdToDict(inserts, uid_value, &pos);
  if (new_inserts == NULL) {
    LCH_LOG_ERROR(
        "Failed to add unique identifier '%s' to inserts for table '%s'.",
        uid_value, self->identifier);
    free(pos);
    free(primary_fields);
    return false;
  }
  if (!self->insert_callback(path, primary_fields, self->subsidiary_fields,
                             new_inserts)) {
    LCH_LOG_ERROR("Failed to patch insertions for table '%s'.",
                  self->identifier);
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
        uid_value, self->identifier);
    free(pos);
    free(primary_fields);
    return false;
  }
  if (!self->delete_callback(path, primary_fields, self->subsidiary_fields,
                             new_deletions)) {
    LCH_LOG_ERROR("Failed to patch deletions for table '%s'.",
                  self->identifier);
    free(primary_fields);
    free(pos);
    LCH_DictDestroy(new_deletions);
    return false;
  }
  LCH_DictDestroy(new_deletions);

  const LCH_Dict *const modifications = LCH_DeltaGetUpdates(patch);
  assert(modifications != NULL);
  LCH_Dict *const new_modifications =
      AddUniqueIdToDict(modifications, uid_value, &pos);
  if (new_modifications == NULL) {
    LCH_LOG_ERROR(
        "Failed to add unique identifier '%s' to modifications for table '%s'.",
        uid_value, self->identifier);
    free(primary_fields);
    free(pos);
    return false;
  }
  if (!self->update_callback(path, primary_fields, self->subsidiary_fields,
                             new_modifications)) {
    LCH_LOG_ERROR("Failed to patch modifications for table '%s'.",
                  self->identifier);
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

void LCH_TableDefinitionDestroy(LCH_TableDefinition *self) { free(self); }
