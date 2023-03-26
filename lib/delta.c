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

struct LCH_Delta {
  const LCH_Table *table;
  LCH_Dict *insert;
  LCH_Dict *delete;
  LCH_Dict *update;
  size_t num_merged;
  size_t num_canceled;
};

LCH_Delta *LCH_DeltaCreate(const LCH_Table *const table,
                           const LCH_Dict *const new_state,
                           const LCH_Dict *const old_state) {
  assert(table != NULL);
  assert((new_state != NULL && old_state != NULL) ||
         (new_state == NULL && old_state == NULL));

  LCH_Delta *delta = malloc(sizeof(LCH_Delta));
  if (delta == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for delta: %s", strerror(errno));
    return NULL;
  }

  delta->table = table;
  delta->num_merged = 0;
  delta->num_canceled = 0;

  const bool create_empty = (new_state == NULL) && (old_state == NULL);

  delta->insert = (create_empty)
                      ? LCH_DictCreate()
                      : LCH_DictSetMinus(new_state, old_state,
                                         (void *(*)(const void *))strdup, free);
  if (delta->insert == NULL) {
    LCH_LOG_ERROR("Failed to compute insertions for delta.");
    free(delta);
    return NULL;
  }

  delta->delete = (create_empty)
                      ? LCH_DictCreate()
                      : LCH_DictSetMinus(old_state, new_state, NULL, NULL);
  if (delta->delete == NULL) {
    LCH_LOG_ERROR("Failed to compute deletions for delta.");
    LCH_DictDestroy(delta->insert);
    free(delta);
    return NULL;
  }

  delta->update =
      (create_empty)
          ? LCH_DictCreate()
          : LCH_DictSetChangedIntersection(
                new_state, old_state, (void *(*)(const void *))strdup, free,
                (int (*)(const void *, const void *))strcmp);
  if (delta->update == NULL) {
    LCH_LOG_ERROR("Failed to compute modifications for delta.");
    LCH_DictDestroy(delta->delete);
    LCH_DictDestroy(delta->insert);
    free(delta);
    return NULL;
  }

  return delta;
}

void LCH_DeltaDestroy(LCH_Delta *const delta) {
  if (delta != NULL) {
    assert(delta->insert != NULL);
    LCH_DictDestroy(delta->insert);

    assert(delta->delete != NULL);
    LCH_DictDestroy(delta->delete);

    assert(delta->update != NULL);
    LCH_DictDestroy(delta->update);

    free(delta);
  }
}

static bool MarshalDeltaOperations(LCH_Buffer *buffer,
                                   const LCH_Table *const table,
                                   const LCH_Dict *const dict) {
  size_t offset;
  if (!LCH_BufferAllocate(buffer, sizeof(uint32_t), &offset)) {
    return false;
  }

  const size_t before = LCH_BufferLength(buffer);

  LCH_List *const records =
      LCH_DictToTable(dict, LCH_TableGetPrimaryFields(table),
                      LCH_TableGetSubsidiaryFields(table), false);
  if (records == NULL) {
    return false;
  }

  if (!LCH_CSVComposeTable(&buffer, records)) {
    free(records);
    return false;
  }
  free(records);

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
  assert(delta->table != NULL);
  assert(delta->insert != NULL);
  assert(delta->delete != NULL);
  assert(delta->update != NULL);

  if (LCH_DictLength(delta->insert) == 0 &&
      LCH_DictLength(delta->delete) == 0 &&
      LCH_DictLength(delta->update) == 0) {
    LCH_LOG_DEBUG("Skipping delta marshaling of table '%s' due to no changes.",
                  LCH_TableGetIdentifier(delta->table));
    return true;
  }

  if (!LCH_MarshalString(buffer, LCH_TableGetIdentifier(delta->table))) {
    LCH_LOG_ERROR("Failed to marshal delta table identifier.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->table, delta->insert)) {
    LCH_LOG_ERROR("Failed to marshal delta insertion operations.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->table, delta->delete)) {
    LCH_LOG_ERROR("Failed to marshal delta deletion operations.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->table, delta->update)) {
    LCH_LOG_ERROR("Failed to marshal delta modification operations.");
    return false;
  }

  return true;
}

static const char *UnmarshalDeltaOperation(LCH_Dict **const dict,
                                           const LCH_Table *const table,
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

  *dict = LCH_TableToDict(records, LCH_TableGetPrimaryFields(table),
                          LCH_TableGetSubsidiaryFields(table), false);
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

  LCH_Delta *const _delta = malloc(sizeof(LCH_Delta));
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

  const LCH_Table *table = LCH_InstanceGetTable(instance, table_id);
  if (table == NULL) {
    LCH_LOG_ERROR("Table with identifier '%s' does not exist.", table_id);
    free(table_id);
    free(_delta);
    return NULL;
  }
  free(table_id);
  _delta->table = table;

  buffer = UnmarshalDeltaOperation(&_delta->insert, _delta->table, buffer);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta insert operations for table '%s'.",
                  LCH_TableGetIdentifier(table));
    free(_delta->insert);
    free(_delta);
    return NULL;
  }

  buffer = UnmarshalDeltaOperation(&_delta->delete, _delta->table, buffer);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta delete operations for table '%s'.",
                  LCH_TableGetIdentifier(table));
    free(_delta->delete);
    free(_delta->insert);
    free(_delta);
    return NULL;
  }

  buffer = UnmarshalDeltaOperation(&_delta->update, _delta->table, buffer);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta update operations for table '%s'.",
                  LCH_TableGetIdentifier(table));
    LCH_DeltaDestroy(_delta);
    return NULL;
  }

  *delta = _delta;
  return buffer;
}

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->insert != NULL);
  return LCH_DictLength(delta->insert);
}

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->delete != NULL);
  return LCH_DictLength(delta->delete);
}

size_t LCH_DeltaGetNumUpdates(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->update != NULL);
  return LCH_DictLength(delta->update);
}

const LCH_Dict *LCH_DeltaGetInsertions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->insert != NULL);
  return delta->insert;
}

const LCH_Dict *LCH_DeltaGetDeletions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->delete != NULL);
  return delta->delete;
}

const LCH_Dict *LCH_DeltaGetUpdates(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->update != NULL);
  return delta->update;
}

const LCH_Table *LCH_DeltaGetTable(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->table != NULL);
  return delta->table;
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
  assert(child->table != NULL);
  assert(child->update != NULL);
  assert(parent->insert != NULL);
  assert(parent->delete != NULL);
  assert(parent->update != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    assert(key != NULL);

    // insert -> insert => error
    if (LCH_DictHasKey(child->insert, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta insert operations for key '%s' in "
          "table '%s'.",
          key, LCH_TableGetIdentifier(child->table));
      return false;
    }

    // insert -> delete => noop
    if (LCH_DictHasKey(child->delete, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'insert -> delete => noop' for key '%s' in table '%s'.",
          key, LCH_TableGetIdentifier(child->table));
      LCH_DictRemove(child->delete, key);
      child->num_canceled += 1;
      continue;
    }

    // insert -> update => insert
    if (LCH_DictHasKey(child->update, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'insert -> update => insert' for key '%s' in table "
          "'%s'.",
          key, LCH_TableGetIdentifier(child->table));
      char *value = LCH_DictRemove(child->update, key);
      assert(value != NULL);
      if (!LCH_DictSet(child->insert, key, value, free)) {
        return false;
      }
      child->num_merged += 1;
      continue;
    }

    // insert -> noop => insert
    char *value = LCH_DictRemove(parent->insert, key);
    if (!LCH_DictSet(child->insert, key, value, free)) {
      return false;
    }
  }

  return true;
}

static bool CompressDeletionOperations(const LCH_List *const keys,
                                       LCH_Delta *const child,
                                       const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(child->table != NULL);
  assert(child->delete != NULL);
  assert(parent->insert != NULL);
  assert(parent->delete != NULL);
  assert(parent->update != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    assert(key != NULL);

    // delete -> insert => update
    if (LCH_DictHasKey(child->insert, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'delete -> insert => update' for key '%s' in table "
          "'%s'.",
          key, LCH_TableGetIdentifier(child->table));
      char *value = LCH_DictRemove(child->insert, key);
      assert(value != NULL);
      if (!LCH_DictSet(child->update, key, value, free)) {
        return false;
      }
      child->num_merged += 1;
      continue;
    }

    // delete -> delete => error
    if (LCH_DictHasKey(child->delete, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta delete operations for key '%s' in table "
          "'%s'.",
          key, LCH_TableGetIdentifier(child->table));
      return false;
    }

    // delete -> update => error
    if (LCH_DictHasKey(child->update, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta delete- and update operations for key "
          "'%s' in table '%s'.",
          key, LCH_TableGetIdentifier(child->table));
      return false;
    }

    // delete -> noop => delete
    char *value = LCH_DictRemove(parent->delete, key);
    if (!LCH_DictSet(child->delete, key, value, free)) {
      return false;
    }
  }

  return true;
}

static bool CompressUpdateOperations(const LCH_List *const keys,
                                     LCH_Delta *const child,
                                     const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(parent->insert != NULL);
  assert(parent->delete != NULL);
  assert(parent->update != NULL);
  assert(child->insert != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    assert(key != NULL);

    // update -> insert => err
    if (LCH_DictHasKey(child->insert, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta update- and insert operations for key "
          "'%s' in table '%s'.",
          key, LCH_TableGetIdentifier(child->table));
      return false;
    }

    // update -> delete => delete
    if (LCH_DictHasKey(child->delete, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'update -> delete => delete' for key '%s' in table "
          "'%s'.",
          key, LCH_TableGetIdentifier(child->table));
      child->num_merged += 1;
      continue;
    }

    // update -> update => update
    if (LCH_DictHasKey(child->update, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'update -> update => update' for key '%s' in table "
          "'%s'.",
          key, LCH_TableGetIdentifier(child->table));
      child->num_merged += 1;
      continue;
    }

    // update -> noop => update
    char *value = LCH_DictRemove(parent->update, key);
    assert(value != NULL);
    if (!LCH_DictSet(child->update, key, value, free)) {
      return false;
    }
  }

  return true;
}

bool LCH_DeltaCompress(LCH_Delta *const child, const LCH_Delta *const parent) {
  assert(child != NULL);
  assert(parent != NULL);
  assert(child->table != NULL);
  assert(parent->table != NULL);
  assert(strcmp(LCH_TableGetIdentifier(child->table),
                LCH_TableGetIdentifier(parent->table)) == 0);

  // child->num_merged = parent->num_merged;
  // child->num_canceled = parent->num_canceled;

  LCH_List *const insert = LCH_DictGetKeys(parent->insert);
  if (insert == NULL) {
    return false;
  }
  if (!CompressInsertionOperations(insert, child, parent)) {
    LCH_LOG_ERROR("Failed to compress insert operations for table '%s'.",
                  LCH_TableGetIdentifier(child->table));
    LCH_ListDestroy(insert);
    return false;
  }
  LCH_ListDestroy(insert);

  LCH_List *const delete = LCH_DictGetKeys(parent->delete);
  if (delete == NULL) {
    return false;
  }
  if (!CompressDeletionOperations(delete, child, parent)) {
    LCH_LOG_ERROR("Failed to compress delete operations for table '%s'.",
                  LCH_TableGetIdentifier(child->table));
    LCH_ListDestroy(delete);
    return false;
  }
  LCH_ListDestroy(delete);

  LCH_List *const update = LCH_DictGetKeys(parent->update);
  if (update == NULL) {
    return false;
  }
  if (!CompressUpdateOperations(update, child, parent)) {
    LCH_LOG_ERROR("Failed to compress update operations for table '%s'.",
                  LCH_TableGetIdentifier(child->table));
    LCH_ListDestroy(update);
    return false;
  }
  LCH_ListDestroy(update);

  return true;
}
