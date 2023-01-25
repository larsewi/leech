#include "delta.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "csv.h"
#include "leech.h"
#include "list.h"

struct LCH_Delta {
  char *table_id;
  LCH_Dict *insertions;
  LCH_Dict *deletions;
  LCH_Dict *modifications;
};

LCH_Delta *LCH_DeltaCreate(const char *const table_id,
                           const LCH_Dict *const new_state,
                           const LCH_Dict *const old_state) {
  assert(table_id != NULL);
  assert((new_state != NULL && old_state != NULL) ||
         (new_state == NULL && old_state == NULL));

  LCH_Delta *delta = malloc(sizeof(LCH_Delta));
  if (delta == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for delta: %s", strerror(errno));
    return NULL;
  }

  delta->table_id = strdup(table_id);
  if (delta->table_id == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for delta table id: '%s'",
                  strerror(errno));
    free(delta);
    return NULL;
  }

  const bool create_empty = (new_state == NULL) && (old_state == NULL);

  delta->insertions =
      (create_empty) ? LCH_DictCreate()
                     : LCH_DictSetMinus(new_state, old_state,
                                        (void *(*)(const void *))strdup, free);
  if (delta->insertions == NULL) {
    LCH_LOG_ERROR("Failed to compute insertions for delta.");
    free(delta->table_id);
    free(delta);
    return NULL;
  }

  delta->deletions =
      (create_empty) ? LCH_DictCreate()
                     : LCH_DictSetMinus(old_state, new_state,
                                        (void *(*)(const void *))strdup, free);
  if (delta->deletions == NULL) {
    LCH_LOG_ERROR("Failed to compute deletions for delta.");
    LCH_DictDestroy(delta->insertions);
    free(delta->table_id);
    free(delta);
    return NULL;
  }

  delta->modifications =
      (create_empty)
          ? LCH_DictCreate()
          : LCH_DictSetChangedIntersection(
                new_state, old_state, (void *(*)(const void *))strdup, free,
                (int (*)(const void *, const void *))strcmp);
  if (delta->modifications == NULL) {
    LCH_LOG_ERROR("Failed to compute modifications for delta.");
    LCH_DictDestroy(delta->deletions);
    LCH_DictDestroy(delta->insertions);
    free(delta->table_id);
    free(delta);
    return NULL;
  }

  return delta;
}

void LCH_DeltaDestroy(LCH_Delta *const delta) {
  if (delta != NULL) {
    assert(delta->insertions != NULL);
    LCH_DictDestroy(delta->insertions);

    assert(delta->deletions != NULL);
    LCH_DictDestroy(delta->deletions);

    assert(delta->modifications != NULL);
    LCH_DictDestroy(delta->modifications);

    assert(delta->table_id != NULL);
    free(delta->table_id);

    free(delta);
  }
}

static bool MarshalTableId(LCH_Buffer *const buffer,
                           const char *const table_id) {
  size_t offset;
  if (!LCH_BufferAllocate(buffer, sizeof(uint32_t), &offset)) {
    return false;
  }

  const size_t before = LCH_BufferLength(buffer);
  if (!LCH_BufferPrintFormat(buffer, table_id)) {
    return false;
  }
  const size_t after = LCH_BufferLength(buffer);

  if (after - before > UINT32_MAX) {
    LCH_LOG_ERROR("Table id too long (%zu > %zu).", after - before, UINT32_MAX);
    return false;
  }

  const uint32_t length = htonl(after - before);
  LCH_BufferSet(buffer, offset, &length, sizeof(uint32_t));

  return true;
}

static bool MarshalDeltaOperations(LCH_Buffer *const buffer,
                                   const LCH_Dict *const dict,
                                   const bool keep_value) {
  size_t offset;
  if (!LCH_BufferAllocate(buffer, sizeof(uint32_t), &offset)) {
    return false;
  }

  const size_t before = LCH_BufferLength(buffer);

  LCH_DictIter *iter = LCH_DictIterCreate(dict);
  if (iter == NULL) {
    LCH_LOG_ERROR("Failed to create iterator for marshaling delta.");
    return NULL;
  }

  while (LCH_DictIterHasNext(iter)) {
    const char *const key = LCH_DictIterGetKey(iter);
    assert(key != NULL);

    if (!LCH_BufferPrintFormat(buffer, "%s\r\n", key)) {
      free(iter);
      return false;
    }

    if (keep_value) {
      const char *const value = (char *)LCH_DictIterGetValue(iter);
      assert(value != NULL);

      if (!LCH_BufferPrintFormat(buffer, "%s\r\n", value)) {
        free(iter);
        return false;
      }
      continue;
    }
  }
  free(iter);

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
  assert(delta->table_id != NULL);
  assert(delta->insertions != NULL);
  assert(delta->deletions != NULL);
  assert(delta->modifications != NULL);

  if (!MarshalTableId(buffer, delta->table_id)) {
    LCH_LOG_ERROR("Failed to marshal delta table id.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->insertions, true)) {
    LCH_LOG_ERROR("Failed to marshal delta insertion operations.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->deletions, false)) {
    LCH_LOG_ERROR("Failed to marshal delta deletion operations.");
    return false;
  }

  if (!MarshalDeltaOperations(buffer, delta->modifications, true)) {
    LCH_LOG_ERROR("Failed to marshal delta modification operations.");
    return false;
  }

  return true;
}

static const char *UnmarshalTableId(LCH_Delta *const delta,
                                    const char *buffer) {
  const uint32_t *const len_ptr = (uint32_t *)buffer;
  const uint32_t length = ntohl(*len_ptr);
  buffer += sizeof(uint32_t);

  delta->table_id = strndup(buffer, length);
  if (delta->table_id == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  buffer += length;
  return buffer;
}

static const char *UnmarshalDeltaOperation(LCH_Dict *const dict,
                                           const char *buffer,
                                           const bool take_value) {
  const uint32_t *const len_ptr = (uint32_t *)buffer;
  const uint32_t length = ntohl(*len_ptr);
  buffer += sizeof(uint32_t);
  if (length == 0) {
    return buffer;
  }

  char data[length + 1];
  memcpy(data, buffer, length);
  data[length] = '\0';

  LCH_List *const table = LCH_CSVParse(data);
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to parse delta operations");
    return NULL;
  }

  char *key = NULL, *value = NULL;
  const size_t n_records = LCH_ListLength(table);
  for (size_t i = 0; i < n_records; i++) {
    LCH_List *const record = LCH_ListGet(table, i);
    assert(record != NULL);

    LCH_Buffer *const comp_buf = LCH_CSVComposeRecord(record);
    if (comp_buf == NULL) {
      free(key);
      free(value);
      LCH_ListDestroy(table);
      return NULL;
    }

    char *const comp_str = LCH_BufferStringDup(comp_buf);
    if (comp_str == NULL) {
      LCH_BufferDestroy(comp_buf);
      free(key);
      free(value);
      LCH_ListDestroy(table);
      return NULL;
    }
    LCH_BufferDestroy(comp_buf);

    if (take_value) {
      if (i % 2 == 0) {
        key = comp_str;
        continue;
      }
      value = comp_str;
    } else {
      key = comp_str;
    }

    if (!LCH_DictSet(dict, key, value, free)) {
      free(key);
      free(value);
      LCH_ListDestroy(table);
      return NULL;
    }
    free(key);
    key = NULL;
    value = NULL;
  }

  LCH_ListDestroy(table);
  buffer += length;
  return buffer;
}

const char *LCH_DeltaUnmarshal(LCH_Delta **delta, const char *buffer) {
  assert(buffer != NULL);

  LCH_Delta *const _delta = malloc(sizeof(LCH_Delta));
  if (_delta == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  buffer = UnmarshalTableId(_delta, buffer);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal table id.");
    free(_delta);
    return NULL;
  }

  _delta->insertions = LCH_DictCreate();
  if (_delta->insertions == NULL) {
    free(_delta->table_id);
    free(_delta);
    return NULL;
  }

  buffer = UnmarshalDeltaOperation(_delta->insertions, buffer, true);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta insertion operations.");
    free(_delta->insertions);
    free(_delta->table_id);
    free(_delta);
    return NULL;
  }

  _delta->deletions = LCH_DictCreate();
  if (_delta->deletions == NULL) {
    free(_delta->insertions);
    free(_delta->table_id);
    free(_delta);
  }

  buffer = UnmarshalDeltaOperation(_delta->deletions, buffer, false);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta deletion operations.");
    free(_delta->deletions);
    free(_delta->insertions);
    free(_delta->table_id);
    free(_delta);
    return NULL;
  }

  _delta->modifications = LCH_DictCreate();
  if (_delta->modifications == NULL) {
    free(_delta->deletions);
    free(_delta->insertions);
    free(_delta->table_id);
    free(_delta);
    return NULL;
  }

  buffer = UnmarshalDeltaOperation(_delta->modifications, buffer, true);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to unmarshal delta modification operations.");
    LCH_DeltaDestroy(_delta);
    return NULL;
  }

  *delta = _delta;
  return buffer;
}

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->insertions != NULL);
  return LCH_DictLength(delta->insertions);
}

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->deletions != NULL);
  return LCH_DictLength(delta->deletions);
}

size_t LCH_DeltaGetNumModifications(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->modifications != NULL);
  return LCH_DictLength(delta->modifications);
}

const char *LCH_DeltaGetTableID(const LCH_Delta *const delta) {
  assert(delta != NULL);
  assert(delta->table_id != NULL);
  return delta->table_id;
}

static bool CompressInsertionOperations(const LCH_List *const keys,
                                        LCH_Delta *const child,
                                        const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(child->table_id != NULL);
  assert(child->modifications != NULL);
  assert(parent->insertions != NULL);
  assert(parent->deletions != NULL);
  assert(parent->modifications != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    assert(key != NULL);

    // insert -> insert => error
    if (LCH_DictHasKey(child->insertions, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta insertion operations for key '%s' in "
          "table '%s'.",
          key, child->table_id);
      return false;
    }

    // insert -> delete => noop
    if (LCH_DictHasKey(child->deletions, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'insert -> delete => noop' for key '%s' in table "
          "'%s'.",
          key, child->table_id);
      char *value = LCH_DictRemove(child->deletions, key);
      assert(value == NULL);
      continue;
    }

    // insert -> modify => insert
    if (LCH_DictHasKey(child->modifications, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'insert -> modify => insert' for key '%s' in table "
          "'%s'.",
          key, child->table_id);

      char *value = LCH_DictRemove(child->modifications, key);
      assert(value != NULL);
      if (!LCH_DictSet(child->insertions, key, value, free)) {
        return false;
      }
      continue;
    }

    LCH_LOG_DEBUG(
        "Compressing 'insert -> noop => insert' for key '%s' in table '%s'.",
        key, child->table_id);
    char *value = LCH_DictRemove(parent->insertions, key);
    assert(value != NULL);
    if (!LCH_DictSet(child->insertions, key, value, free)) {
      return false;
    }
  }

  return true;
}

static bool CompressDeletionOperations(const LCH_List *const keys,
                                       LCH_Delta *const child,
                                       const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(child->table_id != NULL);
  assert(child->deletions != NULL);
  assert(parent->insertions != NULL);
  assert(parent->deletions != NULL);
  assert(parent->modifications != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    assert(key != NULL);

    // delete -> insert => modify
    if (LCH_DictHasKey(child->insertions, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'delete -> insert => modify' for key '%s' in table '%s'.",
          key, child->table_id);
      char *value = LCH_DictRemove(child->insertions, key);
      assert(value != NULL);
      if (!LCH_DictSet(child->modifications, key, value, free)) {
        return false;
      }
      continue;
    }

    // delete -> delete => error
    if (LCH_DictHasKey(child->deletions, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta deletion operations for key '%s' in "
          "table '%s'.",
          key, child->table_id);
      return false;
    }

    // delete -> modify => error
    if (LCH_DictHasKey(child->modifications, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta deletion- and modification operations for key '%s' in "
          "table '%s'.",
          key, child->table_id);
      return false;
    }

    LCH_LOG_DEBUG(
        "Compressing 'delete -> noop => delete' for key '%s' in table '%s'.",
        key, child->table_id);
    char *value = LCH_DictRemove(parent->deletions, key);
    assert(value == NULL);
    if (!LCH_DictSet(child->deletions, key, NULL, NULL)) {
      return false;
    }
  }

  return true;
}

static bool CompressModificationOperations(const LCH_List *const keys,
                                           LCH_Delta *const child,
                                           const LCH_Delta *const parent) {
  assert(keys != NULL);
  assert(parent->insertions != NULL);
  assert(parent->deletions != NULL);
  assert(parent->modifications != NULL);

  size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    assert(key != NULL);

    // modify -> insert => err
    if (LCH_DictHasKey(child->insertions, key)) {
      LCH_LOG_ERROR(
          "Found two subsequent delta modification- and insertion operations for key '%s' in "
          "table '%s'.",
          key, child->table_id);
      return false;
    }

    // modify -> delete => delete
    if (LCH_DictHasKey(child->deletions, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'modify -> delete => delete' for key '%s' in table "
          "'%s'.",
          key, child->table_id);
      continue;
    }

    // modify -> modify => modify
    if (LCH_DictHasKey(child->modifications, key)) {
      LCH_LOG_DEBUG(
          "Compressing 'modify -> modify => modify' for key '%s' in table "
          "'%s'.",
          key, child->table_id);
      continue;
    }

    LCH_LOG_DEBUG(
        "Compressing 'modify -> noop => modify' for key '%s' in table '%s'.",
        key, child->table_id);
    char *value = LCH_DictRemove(parent->modifications, key);
    assert(value != NULL);
    if (!LCH_DictSet(child->modifications, key, value, free)) {
      return false;
    }
  }

  return true;
}

bool LCH_DeltaCompress(LCH_Delta *const child, const LCH_Delta *const parent) {
  assert(child != NULL);
  assert(parent != NULL);
  assert(child->table_id != NULL);
  assert(parent->table_id != NULL);
  assert(strcmp(child->table_id, parent->table_id) == 0);

  LCH_List *const insertions = LCH_DictGetKeys(parent->insertions);
  if (insertions == NULL) {
    return false;
  }

  LCH_List *const deletions = LCH_DictGetKeys(parent->deletions);
  if (deletions == NULL) {
    LCH_ListDestroy(insertions);
    return false;
  }

  LCH_List *const modifications = LCH_DictGetKeys(parent->modifications);
  if (modifications == NULL) {
    LCH_ListDestroy(deletions);
    LCH_ListDestroy(insertions);
    return false;
  }

  if (!CompressInsertionOperations(insertions, child, parent)) {
    LCH_LOG_ERROR("Failed to compress insertion operations for table '%s'.",
                  child->table_id);
    LCH_ListDestroy(modifications);
    LCH_ListDestroy(deletions);
    LCH_ListDestroy(insertions);
    return false;
  }
  LCH_ListDestroy(insertions);

  if (!CompressDeletionOperations(deletions, child, parent)) {
    LCH_LOG_ERROR("Failed to compress deletion operations for table '%s'.",
                  child->table_id);
    LCH_ListDestroy(modifications);
    LCH_ListDestroy(deletions);
    return false;
  }
  LCH_ListDestroy(deletions);

  if (!CompressModificationOperations(modifications, child, parent)) {
    LCH_LOG_ERROR("Failed to compress modification operations for table '%s'.",
                  child->table_id);
    LCH_ListDestroy(modifications);
    return false;
  }
  LCH_ListDestroy(modifications);

  return true;
}

bool LCH_DeltaPatchTable(const LCH_Delta *const delta, LCH_Dict *const table) {
  assert(delta != NULL);
  assert(table != NULL);
  return false;
}
