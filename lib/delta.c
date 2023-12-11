#include "delta.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "csv.h"
#include "instance.h"
#include "leech.h"
#include "list.h"
#include "table.h"
#include "utils.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

struct LCH_Delta {
  const LCH_TableDefinition *table_def;
  LCH_Dict *ins;
  LCH_Dict *del;
  LCH_Dict *upd;
  size_t num_merged;
  size_t num_canceled;
};

static bool DeltaCreateType(LCH_Dict *const delta) {
  char *const type = strdup("delta");
  if (type == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for delta type field");
    return false;
  }

  LCH_Json *const json = LCH_JsonCreateString(type);
  if (json == NULL) {
    free(type);
    return false;
  }

  if (!LCH_DictSet(delta, "type", json, (void (*)(void *))LCH_JsonDestroy)) {
    LCH_JsonDestroy(json);
    return false;
  }

  return true;
}

static bool DeltaCreateVersion(LCH_Dict *const delta) {
  char *const version =
      LCH_VersionToString(VERSION_MAJOR, VERSION_MAJOR, VERSION_PATCH);
  if (version == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for delta version field");
    return false;
  }

  LCH_Json *const json = LCH_JsonCreateString(version);
  if (json == NULL) {
    free(version);
    return false;
  }

  if (!LCH_DictSet(delta, "version", json, (void (*)(void *))LCH_JsonDestroy)) {
    LCH_JsonDestroy(json);
    return false;
  }

  return true;
}

static bool DeltaCreateTableId(LCH_Dict *const delta,
                               const char *const table_id) {
  char *const id = strdup(table_id);
  if (id == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for delta table identifier field");
    return false;
  }

  LCH_Json *const json = LCH_JsonCreateString(id);
  if (json == NULL) {
    free(id);
    return false;
  }

  if (!LCH_DictSet(delta, "table", json, (void (*)(void *))LCH_JsonDestroy)) {
    LCH_JsonDestroy(json);
    return false;
  }

  return true;
}

static bool DeltaCreateInsertOperations(LCH_Dict *const delta,
                                        const LCH_Dict *const new_state,
                                        const LCH_Dict *const old_state) {
  LCH_Dict *const inserts = LCH_DictSetMinus(
      new_state, old_state, (void *(*)(const void *))strdup, free);
  if (inserts == NULL) {
    LCH_LOG_ERROR("Failed to compute insert operations for delta.");
    return false;
  }

  LCH_Json *const json = LCH_JsonCreateObject(inserts);
  if (json == NULL) {
    LCH_DictDestroy(inserts);
    return false;
  }

  if (!LCH_DictSet(delta, "insert", json, (void (*)(void *))LCH_JsonDestroy)) {
    LCH_JsonDestroy(json);
    return false;
  }

  return true;
}

static bool DeltaCreateDeleteOperations(LCH_Dict *const delta,
                                        const LCH_Dict *const new_state,
                                        const LCH_Dict *const old_state) {
  LCH_Dict *const del = LCH_DictSetMinus(old_state, new_state, NULL, NULL);
  if (del == NULL) {
    LCH_LOG_ERROR("Failed to compute delete operations for delta.");
    return false;
  }

  LCH_Json *const json = LCH_JsonCreateObject(del);
  if (json == NULL) {
    LCH_DictDestroy(del);
    return false;
  }

  if (!LCH_DictSet(delta, "delete", json, (void (*)(void *))LCH_JsonDestroy)) {
    LCH_JsonDestroy(json);
    return false;
  }

  return true;
}

static bool DeltaCreateUpdateOperations(LCH_Dict *const delta,
                                        const LCH_Dict *const new_state,
                                        const LCH_Dict *const old_state) {
  LCH_Dict *const upd = LCH_DictSetChangedIntersection(
      new_state, old_state, (void *(*)(const void *))strdup, free,
      (int (*)(const void *, const void *))strcmp);
  if (upd == NULL) {
    LCH_LOG_ERROR("Failed to compute update operations for delta.");
    return false;
  }

  LCH_Json *const json = LCH_JsonCreateObject(upd);
  if (json == NULL) {
    LCH_DictDestroy(upd);
    return false;
  }

  if (!LCH_DictSet(delta, "update", json, (void (*)(void *))LCH_JsonDestroy)) {
    LCH_JsonDestroy(json);
    return false;
  }

  return true;
}
LCH_Json *LCH_DeltaCreateV2(const char *const table_id,
                            const LCH_Dict *const new_state,
                            const LCH_Dict *const old_state) {
  assert(table_id != NULL);
  assert(new_state != NULL);
  assert(old_state != NULL);

  LCH_Dict *const delta = LCH_DictCreate();
  if (delta == NULL) {
    return NULL;
  }

  if (!DeltaCreateType(delta)) {
    LCH_DictDestroy(delta);
    return NULL;
  }

  if (!DeltaCreateVersion(delta)) {
    LCH_DictDestroy(delta);
    return NULL;
  }

  if (!DeltaCreateTableId(delta, table_id)) {
    LCH_DictDestroy(delta);
    return NULL;
  }

  if (!DeltaCreateInsertOperations(delta, new_state, old_state)) {
    LCH_DictDestroy(delta);
    return NULL;
  }

  if (!DeltaCreateDeleteOperations(delta, new_state, old_state)) {
    LCH_DictDestroy(delta);
    return NULL;
  }

  if (!DeltaCreateUpdateOperations(delta, new_state, old_state)) {
    LCH_DictDestroy(delta);
    return NULL;
  }

  LCH_Json *const json = LCH_JsonCreateObject(delta);
  if (json == NULL) {
    LCH_DictDestroy(delta);
    return NULL;
  }

  return json;
}

size_t LCH_DeltaGetNumInsertsV2(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const inserts = LCH_JsonObjectGet(delta, "insert");
  const size_t num_inserts = LCH_JsonObjectLength(inserts);
  return num_inserts;
}

size_t LCH_DeltaGetNumDeletesV2(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const deletes = LCH_JsonObjectGet(delta, "delete");
  const size_t num_deletes = LCH_JsonObjectLength(deletes);
  return num_deletes;
}

size_t LCH_DeltaGetNumUpdatesV2(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const updates = LCH_JsonObjectGet(delta, "update");
  const size_t num_updates = LCH_JsonObjectLength(updates);
  return num_updates;
}

const char *LCH_DeltaGetTableIDV2(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const table_id = LCH_JsonObjectGet(delta, "table");
  const char *const str = LCH_JsonStringGet(table_id);
  return str;
}

LCH_Delta *LCH_DeltaCreate(const LCH_TableDefinition *const table_def,
                           const LCH_Dict *const new_state,
                           const LCH_Dict *const old_state) {
  assert(table_def != NULL);
  assert((new_state != NULL && old_state != NULL) ||
         (new_state == NULL && old_state == NULL));

  LCH_Delta *delta = (LCH_Delta *)malloc(sizeof(LCH_Delta));
  if (delta == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for delta: %s", strerror(errno));
    return NULL;
  }

  delta->table_def = table_def;
  delta->num_merged = 0;
  delta->num_canceled = 0;

  const bool create_empty = (new_state == NULL) && (old_state == NULL);

  delta->ins = (create_empty)
                   ? LCH_DictCreate()
                   : LCH_DictSetMinus(new_state, old_state,
                                      (void *(*)(const void *))strdup, free);
  if (delta->ins == NULL) {
    LCH_LOG_ERROR("Failed to compute insertions for delta.");
    free(delta);
    return NULL;
  }

  delta->del = (create_empty)
                   ? LCH_DictCreate()
                   : LCH_DictSetMinus(old_state, new_state, NULL, NULL);
  if (delta->del == NULL) {
    LCH_LOG_ERROR("Failed to compute deletions for delta.");
    LCH_DictDestroy(delta->ins);
    free(delta);
    return NULL;
  }

  delta->upd = (create_empty)
                   ? LCH_DictCreate()
                   : LCH_DictSetChangedIntersection(
                         new_state, old_state, (void *(*)(const void *))strdup,
                         free, (int (*)(const void *, const void *))strcmp);
  if (delta->upd == NULL) {
    LCH_LOG_ERROR("Failed to compute modifications for delta.");
    LCH_DictDestroy(delta->del);
    LCH_DictDestroy(delta->ins);
    free(delta);
    return NULL;
  }

  return delta;
}

void LCH_DeltaDestroy(LCH_Delta *const delta) {
  if (delta != NULL) {
    assert(delta->ins != NULL);
    LCH_DictDestroy(delta->ins);

    assert(delta->del != NULL);
    LCH_DictDestroy(delta->del);

    assert(delta->upd != NULL);
    LCH_DictDestroy(delta->upd);

    free(delta);
  }
}

static bool MarshalDeltaOperations(LCH_Buffer *buffer,
                                   const LCH_TableDefinition *const table_def,
                                   const LCH_Dict *const dict) {
  size_t offset;
  if (!LCH_BufferAllocate(buffer, sizeof(uint32_t), &offset)) {
    return false;
  }

  const size_t before = LCH_BufferLength(buffer);

  LCH_List *const records =
      LCH_DictToTable(dict, LCH_TableDefinitionGetPrimaryFields(table_def),
                      LCH_TableDefinitionGetSubsidiaryFields(table_def), false);
  if (records == NULL) {
    return false;
  }

  if (!LCH_CSVComposeTable(&buffer, records)) {
    LCH_ListDestroy(records);
    return false;
  }
  LCH_ListDestroy(records);

  const size_t after = LCH_BufferLength(buffer);
  if (after - before > UINT32_MAX) {
    return false;
  }

  const uint32_t length = htonl(after - before);
  LCH_BufferSet(buffer, offset, &length, sizeof(uint32_t));

  return true;
}

bool LCH_DeltaMarshal(LCH_Buffer *const buffer, const LCH_Delta *const delta) {
  assert(buffer != NULL);
  assert(delta != NULL);
  assert(delta->table_def != NULL);
  assert(delta->ins != NULL);
  assert(delta->del != NULL);
  assert(delta->upd != NULL);

  if (LCH_DictLength(delta->ins) == 0 && LCH_DictLength(delta->del) == 0 &&
      LCH_DictLength(delta->upd) == 0) {
    LCH_LOG_DEBUG("Skipping delta marshaling of table '%s' due to no changes.",
                  LCH_TableDefinitionGetIdentifier(delta->table_def));
    return true;
  }

  if (!LCH_MarshalString(buffer,
                         LCH_TableDefinitionGetIdentifier(delta->table_def))) {
    LCH_LOG_ERROR("Failed to marshal delta table identifier.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->table_def, delta->ins)) {
    LCH_LOG_ERROR("Failed to marshal delta insertion operations.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->table_def, delta->del)) {
    LCH_LOG_ERROR("Failed to marshal delta deletion operations.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->table_def, delta->upd)) {
    LCH_LOG_ERROR("Failed to marshal delta modification operations.");
    return false;
  }

  return true;
}

static const char *UnmarshalDeltaOperation(
    LCH_Dict **const dict, const LCH_TableDefinition *const table_def,
    const char *buffer) {
  char *csv;
  buffer = LCH_UnmarshalBinary(buffer, &csv);
  if (buffer == NULL) {
    return NULL;
  }

  LCH_List *const records =
      (strcmp(csv, "") == 0) ? LCH_ListCreate() : LCH_CSVParseTable(csv);
  free(csv);
  if (records == NULL) {
    return NULL;
  }

  *dict =
      LCH_TableToDict(records, LCH_TableDefinitionGetPrimaryFields(table_def),
                      LCH_TableDefinitionGetSubsidiaryFields(table_def), false);
  LCH_ListDestroy(records);
  if (*dict == NULL) {
    return NULL;
  }

  return buffer;
}

const char *LCH_DeltaUnmarshal(LCH_Delta **delta,
                               const LCH_Instance *const instance,
                               const char *buffer) {
  assert(buffer != NULL);

  LCH_Delta *const _delta = (LCH_Delta *)malloc(sizeof(LCH_Delta));
  if (_delta == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  char *table_id;
  buffer = LCH_UnmarshalString(buffer, &table_id);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal table id.");
    free(_delta);
    return NULL;
  }

  const LCH_TableDefinition *table_def =
      LCH_InstanceGetTable(instance, table_id);
  if (table_def == NULL) {
    LCH_LOG_ERROR("Table with identifier '%s' does not exist.", table_id);
    free(table_id);
    free(_delta);
    return NULL;
  }
  free(table_id);
  _delta->table_def = table_def;

  buffer = UnmarshalDeltaOperation(&_delta->ins, _delta->table_def, buffer);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta insert operations for table '%s'.",
                  LCH_TableDefinitionGetIdentifier(table_def));
    free(_delta->ins);
    free(_delta);
    return NULL;
  }

  buffer = UnmarshalDeltaOperation(&_delta->del, _delta->table_def, buffer);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta delete operations for table '%s'.",
                  LCH_TableDefinitionGetIdentifier(table_def));
    free(_delta->del);
    free(_delta->ins);
    free(_delta);
    return NULL;
  }

  buffer = UnmarshalDeltaOperation(&_delta->upd, _delta->table_def, buffer);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta update operations for table '%s'.",
                  LCH_TableDefinitionGetIdentifier(table_def));
    LCH_DeltaDestroy(_delta);
    return NULL;
  }

  *delta = _delta;
  return buffer;
}

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->ins != NULL);
  return LCH_DictLength(delta->ins);
}

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->del != NULL);
  return LCH_DictLength(delta->del);
}

size_t LCH_DeltaGetNumUpdates(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->upd != NULL);
  return LCH_DictLength(delta->upd);
}

const LCH_Dict *LCH_DeltaGetInsertions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->ins != NULL);
  return delta->ins;
}

const LCH_Dict *LCH_DeltaGetDeletions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->del != NULL);
  return delta->del;
}

const LCH_Dict *LCH_DeltaGetUpdates(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->upd != NULL);
  return delta->upd;
}

const LCH_TableDefinition *LCH_DeltaGetTable(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->table_def != NULL);
  return delta->table_def;
}

size_t LCH_DeltaGetNumMergedOperations(const LCH_Delta *const delta) {
  assert(delta != NULL);
  return delta->num_merged;
}

size_t LCH_DeltaGetNumCanceledOperations(const LCH_Delta *const delta) {
  assert(delta != NULL);
  return delta->num_canceled;
}

static bool CompressInsertionOperations(const LCH_List *const keys,
                                        LCH_Delta *const child,
                                        const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(child->table_def != NULL);
  assert(child->upd != NULL);
  assert(parent->ins != NULL);
  assert(parent->del != NULL);
  assert(parent->upd != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    // insert -> insert => error
    if (LCH_DictHasKey(child->ins, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta insert operations for key '%s' in "
          "table '%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      return false;
    }

    // insert -> delete => noop
    if (LCH_DictHasKey(child->del, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'insert -> delete => noop' for key '%s' in table '%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      LCH_DictRemove(child->del, key);
      child->num_canceled += 1;
      continue;
    }

    // insert -> update => insert
    if (LCH_DictHasKey(child->upd, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'insert -> update => insert' for key '%s' in table "
          "'%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      char *value = (char *)LCH_DictRemove(child->upd, key);
      assert(value != NULL);
      if (!LCH_DictSet(child->ins, key, value, free)) {
        return false;
      }
      child->num_merged += 1;
      continue;
    }

    // insert -> noop => insert
    char *value = (char *)LCH_DictRemove(parent->ins, key);
    if (!LCH_DictSet(child->ins, key, value, free)) {
      return false;
    }
  }

  return true;
}

static bool CompressDeletionOperations(const LCH_List *const keys,
                                       LCH_Delta *const child,
                                       const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(child->table_def != NULL);
  assert(child->del != NULL);
  assert(parent->ins != NULL);
  assert(parent->del != NULL);
  assert(parent->upd != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    // delete -> insert => update
    if (LCH_DictHasKey(child->ins, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'delete -> insert => update' for key '%s' in table "
          "'%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      char *value = (char *)LCH_DictRemove(child->ins, key);
      assert(value != NULL);
      if (!LCH_DictSet(child->upd, key, value, free)) {
        return false;
      }
      child->num_merged += 1;
      continue;
    }

    // delete -> delete => error
    if (LCH_DictHasKey(child->del, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta delete operations for key '%s' in table "
          "'%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      return false;
    }

    // delete -> update => error
    if (LCH_DictHasKey(child->upd, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta delete- and update operations for key "
          "'%s' in table '%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      return false;
    }

    // delete -> noop => delete
    char *value = (char *)LCH_DictRemove(parent->del, key);
    if (!LCH_DictSet(child->del, key, value, free)) {
      return false;
    }
  }

  return true;
}

static bool CompressUpdateOperations(const LCH_List *const keys,
                                     LCH_Delta *const child,
                                     const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(parent->ins != NULL);
  assert(parent->del != NULL);
  assert(parent->upd != NULL);
  assert(child->ins != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    // update -> insert => err
    if (LCH_DictHasKey(child->ins, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta update- and insert operations for key "
          "'%s' in table '%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      return false;
    }

    // update -> delete => delete
    if (LCH_DictHasKey(child->del, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'update -> delete => delete' for key '%s' in table "
          "'%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      child->num_merged += 1;
      continue;
    }

    // update -> update => update
    if (LCH_DictHasKey(child->upd, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'update -> update => update' for key '%s' in table "
          "'%s'.",
          key, LCH_TableDefinitionGetIdentifier(child->table_def));
      child->num_merged += 1;
      continue;
    }

    // update -> noop => update
    char *value = (char *)LCH_DictRemove(parent->upd, key);
    assert(value != NULL);
    if (!LCH_DictSet(child->upd, key, value, free)) {
      return false;
    }
  }

  return true;
}

bool LCH_DeltaCompress(LCH_Delta *const child, const LCH_Delta *const parent) {
  assert(child != NULL);
  assert(parent != NULL);
  assert(child->table_def != NULL);
  assert(parent->table_def != NULL);
  assert(strcmp(LCH_TableDefinitionGetIdentifier(child->table_def),
                LCH_TableDefinitionGetIdentifier(parent->table_def)) == 0);

  // child->num_merged = parent->num_merged;
  // child->num_canceled = parent->num_canceled;

  LCH_List *const ins = LCH_DictGetKeys(parent->ins);
  if (ins == NULL) {
    return false;
  }
  if (!CompressInsertionOperations(ins, child, parent)) {
    LCH_LOG_ERROR("Failed to compress insert operations for table '%s'.",
                  LCH_TableDefinitionGetIdentifier(child->table_def));
    LCH_ListDestroy(ins);
    return false;
  }
  LCH_ListDestroy(ins);

  LCH_List *const del = (LCH_List *)LCH_DictGetKeys(parent->del);
  if (del == NULL) {
    return false;
  }
  if (!CompressDeletionOperations(del, child, parent)) {
    LCH_LOG_ERROR("Failed to compress delete operations for table '%s'.",
                  LCH_TableDefinitionGetIdentifier(child->table_def));
    LCH_ListDestroy(del);
    return false;
  }
  LCH_ListDestroy(del);

  LCH_List *const upd = LCH_DictGetKeys(parent->upd);
  if (upd == NULL) {
    return false;
  }
  if (!CompressUpdateOperations(upd, child, parent)) {
    LCH_LOG_ERROR("Failed to compress update operations for table '%s'.",
                  LCH_TableDefinitionGetIdentifier(child->table_def));
    LCH_ListDestroy(upd);
    return false;
  }
  LCH_ListDestroy(upd);

  return true;
}
