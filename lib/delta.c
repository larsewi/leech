#include "delta.h"

#include <assert.h>

#include "logger.h"
#include "utils.h"

LCH_Json *LCH_DeltaCreate(const char *const table_id, const char *const type,
                          const LCH_Json *const new_state,
                          const LCH_Json *const old_state) {
  assert(table_id != NULL);
  assert(type != NULL);
  assert(new_state != NULL);
  assert(old_state != NULL);

  LCH_Json *const delta = LCH_JsonObjectCreate();
  if (delta == NULL) {
    return NULL;
  }

  {  // type
    LCH_Buffer *const value = LCH_BufferFromString(type);
    if (value == NULL) {
      LCH_JsonDestroy(delta);
      return NULL;
    }

    const LCH_Buffer key = LCH_BufferStaticFromString("type");
    if (!LCH_JsonObjectSetString(delta, &key, value)) {
      LCH_BufferDestroy(value);
      LCH_JsonDestroy(delta);
      return NULL;
    }
  }

  {  // id
    LCH_Buffer *const value = LCH_BufferFromString(table_id);
    if (value == NULL) {
      LCH_JsonDestroy(delta);
      return NULL;
    }

    const LCH_Buffer key = LCH_BufferStaticFromString("id");
    if (!LCH_JsonObjectSetString(delta, &key, value)) {
      LCH_BufferDestroy(value);
      LCH_JsonDestroy(delta);
      return NULL;
    }
  }

  {  // inserts
    LCH_Json *const value = LCH_JsonObjectKeysSetMinus(new_state, old_state);
    if (value == NULL) {
      LCH_JsonDestroy(delta);
      return NULL;
    }

    const LCH_Buffer key = LCH_BufferStaticFromString("inserts");
    if (!LCH_JsonObjectSet(delta, &key, value)) {
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(delta);
      return NULL;
    }
  }

  {  // deletes
    LCH_Json *const value = LCH_JsonObjectKeysSetMinus(old_state, new_state);
    if (value == NULL) {
      LCH_JsonDestroy(delta);
      return NULL;
    }

    const LCH_Buffer key = LCH_BufferStaticFromString("deletes");
    if (!LCH_JsonObjectSet(delta, &key, value)) {
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(delta);
      return NULL;
    }
  }

  // updates
  {
    LCH_Json *const value =
        LCH_JsonObjectKeysSetIntersectAndValuesSetMinus(new_state, old_state);
    if (value == NULL) {
      LCH_JsonDestroy(delta);
      return NULL;
    }

    const LCH_Buffer key = LCH_BufferStaticFromString("updates");
    if (!LCH_JsonObjectSet(delta, &key, value)) {
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(delta);
      return NULL;
    }
  }

  return delta;
}

const char *LCH_DeltaGetTableId(const LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("id");
  const LCH_Buffer *const value = LCH_JsonObjectGetString(delta, &key);
  if (value == NULL) {
    return NULL;
  }
  return LCH_BufferData(value);
}

const char *LCH_DeltaGetType(const LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("type");
  const LCH_Buffer *const value = LCH_JsonObjectGetString(delta, &key);
  if (value == NULL) {
    return NULL;
  }
  return LCH_BufferData(value);
}

const LCH_Json *LCH_DeltaGetInserts(const LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("inserts");
  const LCH_Json *const value = LCH_JsonObjectGetObject(delta, &key);
  return value;
}

const LCH_Json *LCH_DeltaGetDeletes(const LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("deletes");
  const LCH_Json *const value = LCH_JsonObjectGetObject(delta, &key);
  return value;
}

const LCH_Json *LCH_DeltaGetUpdates(const LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("updates");
  const LCH_Json *const value = LCH_JsonObjectGetObject(delta, &key);
  return value;
}

bool LCH_DeltaGetNumOperations(const LCH_Json *const delta,
                               size_t *const num_inserts,
                               size_t *const num_deletes,
                               size_t *const num_updates) {
  if (num_inserts != NULL) {
    const LCH_Json *const inserts = LCH_DeltaGetInserts(delta);
    if (inserts == NULL) {
      return false;
    }
    *num_inserts = LCH_JsonObjectLength(inserts);
  }

  if (num_deletes != NULL) {
    const LCH_Json *const deletes = LCH_DeltaGetDeletes(delta);
    if (deletes == NULL) {
      return false;
    }
    *num_deletes = LCH_JsonObjectLength(deletes);
  }

  if (num_updates != NULL) {
    const LCH_Json *const updates = LCH_DeltaGetUpdates(delta);
    if (updates == NULL) {
      return false;
    }
    *num_updates = LCH_JsonObjectLength(updates);
  }

  return true;
}

static LCH_Json *DeltaRemoveInserts(LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("inserts");
  LCH_Json *const value = LCH_JsonObjectRemoveObject(delta, &key);
  return value;
}

static LCH_Json *DeltaRemoveDeletes(LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("deletes");
  LCH_Json *const value = LCH_JsonObjectRemoveObject(delta, &key);
  return value;
}

static LCH_Json *DeltaRemoveUpdates(LCH_Json *const delta) {
  const LCH_Buffer key = LCH_BufferStaticFromString("updates");
  LCH_Json *const value = LCH_JsonObjectRemoveObject(delta, &key);
  return value;
}

static bool MergeInsertOperations(const LCH_Json *const parent,
                                  LCH_Json *const child_inserts) {
  assert(parent != NULL);
  assert(child_inserts != NULL);

  const LCH_Json *const parent_inserts = LCH_DeltaGetInserts(parent);
  if (parent_inserts == NULL) {
    return false;
  }

  const LCH_Json *const parent_deletes = LCH_DeltaGetDeletes(parent);
  if (parent_deletes == NULL) {
    return false;
  }

  const LCH_Json *const parent_updates = LCH_DeltaGetUpdates(parent);
  if (parent_updates == NULL) {
    return false;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(child_inserts);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);

    char *const printable_key = LCH_BufferToPrintable(key);
    if (printable_key == NULL) {
      /* Error is already logged */
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_inserts, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with insert operations into parent
      //////////////////////////////////////////////////////////////////////////

      // parent_insert(key, val) -> child_insert(key, val) => ERROR
      LCH_LOG_ERROR(
          "Found two subsequent insert operations on the same key (key=%s)",
          printable_key);
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_deletes, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with delete operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_Json *const parent_value = LCH_JsonObjectRemove(parent_deletes, key);
      if (parent_value == NULL) {
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      LCH_Json *const child_value = LCH_JsonObjectRemove(child_inserts, key);
      if (child_value == NULL) {
        LCH_JsonDestroy(parent_value);
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      const bool is_equal = LCH_JsonEqual(parent_value, child_value);
      LCH_JsonDestroy(parent_value);

      if (is_equal) {
        LCH_LOG_DEBUG(
            "Merging: delete(key, val) -> insert(key, val) => NOOP "
            "(key=%s)",
            printable_key);
        LCH_JsonDestroy(child_value);
      } else {
        LCH_LOG_DEBUG(
            "Merging: delete(key, val1) -> insert(key, val2) => update(key, "
            "val2) (key=%s)",
            printable_key);

        if (!LCH_JsonObjectSet(parent_updates, key, child_value)) {
          LCH_JsonDestroy(child_value);
          free(printable_key);
          LCH_ListDestroy(keys);
          return false;
        }
      }

      free(printable_key);
      continue;
    }

    if (LCH_JsonObjectHasKey(parent_updates, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with update operations in parent
      //////////////////////////////////////////////////////////////////////////

      // parent_update(key, val) -> child_insert(key, val) => ERROR
      LCH_LOG_ERROR(
          "Found an update operation followed by an insert operation on the "
          "same key (key=%s)",
          printable_key);
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_LOG_DEBUG(
        "Merging: NOOP -> insert(key, val) => insert(key, val) (key=%s)",
        printable_key);
    LCH_Json *const child_value = LCH_JsonObjectRemove(child_inserts, key);
    if (child_value == NULL) {
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!LCH_JsonObjectSet(parent_inserts, key, child_value)) {
      LCH_JsonDestroy(child_value);
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    free(printable_key);
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool MergeDeleteOperations(const LCH_Json *const parent,
                                  LCH_Json *const child_deletes) {
  assert(parent != NULL);
  assert(child_deletes != NULL);

  const LCH_Json *const parent_inserts = LCH_DeltaGetInserts(parent);
  if (parent_inserts == NULL) {
    return false;
  }

  const LCH_Json *const parent_deletes = LCH_DeltaGetDeletes(parent);
  if (parent_deletes == NULL) {
    return false;
  }

  const LCH_Json *const parent_updates = LCH_DeltaGetUpdates(parent);
  if (parent_updates == NULL) {
    return false;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(child_deletes);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);

    char *const printable_key = LCH_BufferToPrintable(key);
    if (printable_key == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_inserts, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with insert operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_Json *const parent_value = LCH_JsonObjectRemove(parent_inserts, key);
      if (parent_value == NULL) {
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      LCH_Json *const child_value = LCH_JsonObjectRemove(child_deletes, key);
      if (child_value == NULL) {
        LCH_JsonDestroy(parent_value);
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      const bool is_equal = LCH_JsonEqual(parent_value, child_value);
      LCH_JsonDestroy(parent_value);

      const bool is_null = LCH_JsonIsNull(child_value);
      LCH_JsonDestroy(child_value);

      if (is_equal) {
        LCH_LOG_DEBUG(
            "Merging: insert(key, val) -> delete(key, val) => NOOP "
            "(key=%s)",
            printable_key);
      } else if (!is_null) {
        // insert(key, val1) -> delete(key, val2) => ERROR
        LCH_LOG_ERROR(
            "Found insert operation followed by delete operation on the same "
            "key, but with different values (key=%s)",
            printable_key);
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      } else {
        LCH_LOG_DEBUG(
            "Merging: insert(key, val) -> delete(key, null) => NOOP "
            "(key=%s)",
            printable_key);
      }
      free(printable_key);
      continue;
    }

    if (LCH_JsonObjectHasKey(parent_deletes, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with delete operations in parent
      //////////////////////////////////////////////////////////////////////////

      // parent_delete(key, val) -> child_delete(key, val) => ERROR
      LCH_LOG_ERROR(
          "Found two subsequent delete operations on the same key (key=%s)",
          printable_key);
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_updates, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with update operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_Json *const parent_value = LCH_JsonObjectRemove(parent_updates, key);
      if (parent_value == NULL) {
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      LCH_Json *const child_value = LCH_JsonObjectRemove(child_deletes, key);
      if (child_value == NULL) {
        LCH_JsonDestroy(parent_value);
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      const bool is_equal = LCH_JsonEqual(parent_value, child_value);
      LCH_JsonDestroy(parent_value);

      const bool is_null = LCH_JsonIsNull(child_value);
      LCH_JsonDestroy(child_value);

      if (is_equal) {
        LCH_LOG_DEBUG(
            "Merging: update(key, val) -> delete(key, val) => delete(key, "
            "null) (key=%s)",
            printable_key);
      } else if (!is_null) {
        LCH_LOG_ERROR(
            "Found an update operation followed by a delete operation on the "
            "same key, but with different values (key=%s)",
            printable_key);
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      } else {
        LCH_LOG_DEBUG(
            "Merging: update(key, val) -> delete(key, null) => delete(key, "
            "null) (key=%s)",
            printable_key);
      }

      /* We have to use null as place holder, because we have no clue what the
       * value was before the update in the child block. This null value will be
       * carried down the chain and will end up in the final patch. However it
       * does not matter, as only the key is required in a delete operation. */
      LCH_Json *const null = LCH_JsonNullCreate();
      if (null == NULL) {
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      if (!LCH_JsonObjectSet(parent_deletes, key, null)) {
        LCH_JsonDestroy(null);
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      free(printable_key);
      continue;
    }

    LCH_LOG_DEBUG(
        "Merging: NOOP -> delete(key, val) => delete(key, val) (key=%s)",
        printable_key);
    LCH_Json *const child_value = LCH_JsonObjectRemove(child_deletes, key);
    if (!LCH_JsonObjectSet(parent_deletes, key, child_value)) {
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    free(printable_key);
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool MergeUpdateOperations(const LCH_Json *const parent,
                                  LCH_Json *const child_updates) {
  assert(parent != NULL);
  assert(child_updates != NULL);

  const LCH_Json *const parent_inserts = LCH_DeltaGetInserts(parent);
  if (parent_inserts == NULL) {
    return false;
  }

  const LCH_Json *const parent_deletes = LCH_DeltaGetDeletes(parent);
  if (parent_deletes == NULL) {
    return false;
  }

  const LCH_Json *const parent_updates = LCH_DeltaGetUpdates(parent);
  if (parent_updates == NULL) {
    return false;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(child_updates);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);

    char *const printable_key = LCH_BufferToPrintable(key);
    if (printable_key == NULL) {
      /* Error already logged */
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_inserts, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with insert operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_LOG_DEBUG(
          "Merging: insert(key, val1) -> update(key, val2) => insert(key, "
          "val2) (key=%s)",
          printable_key);

      LCH_Json *const child_value = LCH_JsonObjectRemove(child_updates, key);
      if (!LCH_JsonObjectSet(parent_inserts, key, child_value)) {
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      free(printable_key);
      continue;
    }

    if (LCH_JsonObjectHasKey(parent_deletes, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with delete operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_LOG_DEBUG(
          "Found a delete block followed by an update operation on the same "
          "key (key=%s)",
          printable_key);
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_updates, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with update operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_LOG_DEBUG(
          "Merging: update(key, val1) -> update(key, val2) => update(key, "
          "val2) (key=%s)",
          printable_key);

      LCH_Json *const child_value = LCH_JsonObjectRemove(child_updates, key);
      if (!LCH_JsonObjectSet(parent_updates, key, child_value)) {
        LCH_JsonDestroy(child_value);
        free(printable_key);
        LCH_ListDestroy(keys);
        return false;
      }

      free(printable_key);
      continue;
    }

    LCH_LOG_DEBUG(
        "Merging: NOOP -> update(key, val) => update(key, val) (key=%s)",
        printable_key);
    LCH_Json *const child_value = LCH_JsonObjectRemove(child_updates, key);
    if (!LCH_JsonObjectSet(parent_updates, key, child_value)) {
      LCH_JsonDestroy(child_value);
      free(printable_key);
      LCH_ListDestroy(keys);
      return false;
    }

    free(printable_key);
  }

  LCH_ListDestroy(keys);
  return true;
}

bool LCH_DeltaMerge(const LCH_Json *const parent, LCH_Json *const child) {
  // Merge child block's inserts into parent block.
  LCH_Json *const child_inserts = DeltaRemoveInserts(child);
  if (child_inserts == NULL) {
    return false;
  }
  if (!MergeInsertOperations(parent, child_inserts)) {
    LCH_JsonDestroy(child_inserts);
    return false;
  }
  LCH_JsonDestroy(child_inserts);

  // Merge child block's deletes into parent block.
  LCH_Json *const child_deletes = DeltaRemoveDeletes(child);
  if (child_deletes == NULL) {
    return false;
  }
  if (!MergeDeleteOperations(parent, child_deletes)) {
    LCH_JsonDestroy(child_deletes);
    return false;
  }
  LCH_JsonDestroy(child_deletes);

  // Merge child block's updates into parent block.
  LCH_Json *const child_updates = DeltaRemoveUpdates(child);
  if (child_updates == NULL) {
    return false;
  }
  if (!MergeUpdateOperations(parent, child_updates)) {
    LCH_JsonDestroy(child_updates);
    return false;
  }
  LCH_JsonDestroy(child_updates);

  return true;
}
