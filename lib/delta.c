#include "delta.h"

#include <assert.h>
#include <string.h>

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

  if (!LCH_JsonObjectSetStringDuplicate(delta, "type", type)) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(delta, "id", table_id)) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  LCH_Json *const inserts = LCH_JsonObjectKeysSetMinus(new_state, old_state);
  if (inserts == NULL) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSet(delta, "inserts", inserts)) {
    LCH_JsonDestroy(inserts);
    LCH_JsonDestroy(delta);
    return NULL;
  }

  LCH_Json *const deletes = LCH_JsonObjectKeysSetMinus(old_state, new_state);
  if (deletes == NULL) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSet(delta, "deletes", deletes)) {
    LCH_JsonDestroy(deletes);
    LCH_JsonDestroy(delta);
    return NULL;
  }

  LCH_Json *const updates =
      LCH_JsonObjectKeysSetIntersectAndValuesSetMinus(new_state, old_state);
  if (updates == NULL) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSet(delta, "updates", updates)) {
    LCH_JsonDestroy(updates);
    LCH_JsonDestroy(delta);
    return NULL;
  }

  return delta;
}

size_t LCH_DeltaGetNumInserts(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const inserts = LCH_JsonObjectGet(delta, "inserts");
  const size_t num_inserts = LCH_JsonObjectLength(inserts);
  return num_inserts;
}

size_t LCH_DeltaGetNumDeletes(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const deletes = LCH_JsonObjectGet(delta, "deletes");
  const size_t num_deletes = LCH_JsonObjectLength(deletes);
  return num_deletes;
}

size_t LCH_DeltaGetNumUpdates(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const updates = LCH_JsonObjectGet(delta, "updates");
  const size_t num_updates = LCH_JsonObjectLength(updates);
  return num_updates;
}

const char *LCH_DeltaGetTableID(const LCH_Json *const delta) {
  assert(delta != NULL);
  const LCH_Json *const table_id = LCH_JsonObjectGet(delta, "id");
  const char *const str = LCH_JsonStringGetString(table_id);
  return str;
}

static bool MergeInsertOperations(const LCH_Json *const parent,
                                  LCH_Json *const child_inserts) {
  assert(parent != NULL);
  assert(child_inserts != NULL);

  const LCH_Json *const parent_inserts =
      LCH_JsonObjectGetObject(parent, "inserts");
  assert(parent_inserts != NULL);

  const LCH_Json *const parent_deletes =
      LCH_JsonObjectGetObject(parent, "deletes");
  assert(parent_deletes != NULL);

  const LCH_Json *const parent_updates =
      LCH_JsonObjectGetObject(parent, "updates");
  assert(parent_updates != NULL);

  LCH_List *const keys = LCH_JsonObjectGetKeys(child_inserts);
  if (keys == NULL) {
    LCH_LOG_ERROR("Failed to get keys from child block's insert operations");
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (const char *)LCH_ListGet(keys, i);

    if (LCH_JsonObjectHasKey(parent_inserts, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with insert operations in parent:
      //////////////////////////////////////////////////////////////////////////

      // parent_insert(key, val) -> child_insert(key, val) => ERROR
      LCH_LOG_ERROR(
          "Found two subsequent insert operations with the same key in parent "
          "and child block (key=\"%s\")",
          key);
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_deletes, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with delete operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_Json *const parent_value = LCH_JsonObjectRemove(parent_deletes, key);
      LCH_Json *const child_value = LCH_JsonObjectRemove(child_inserts, key);

      const bool is_equal = LCH_JsonIsEqual(parent_value, child_value);
      LCH_JsonDestroy(parent_value);

      if (is_equal) {
        LCH_LOG_DEBUG(
            "Merging: delete(key, val) -> insert(key, val) => NOOP "
            "(key=\"%s\")",
            key);
        LCH_JsonDestroy(child_value);
      } else {
        LCH_LOG_DEBUG(
            "Merging: delete(key, val1) -> insert(key, val2) => update(key, "
            "val2) (key=\"%s\")",
            key);

        if (!LCH_JsonObjectSet(parent_updates, key, child_value)) {
          LCH_LOG_ERROR(
              "Failed to insert operation from child block into update "
              "operations in parent block (key=\"%s\")",
              key);
          LCH_JsonDestroy(child_value);
          LCH_ListDestroy(keys);
          return false;
        }
      }
      continue;
    }

    if (LCH_JsonObjectHasKey(parent_updates, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with update operations in parent
      //////////////////////////////////////////////////////////////////////////

      // parent_update(key, val) -> child_insert(key, val) => ERROR
      LCH_LOG_ERROR(
          "Found an update operation in parent block followed by an insert "
          "operation in child block (key=\"%s\")",
          key);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_LOG_DEBUG(
        "Merging: NOOP -> insert(key, val) => insert(key, val) (key=\"%s\")",
        key);
    LCH_Json *const child_value = LCH_JsonObjectRemove(child_inserts, key);
    if (!LCH_JsonObjectSet(parent_inserts, key, child_value)) {
      LCH_LOG_ERROR(
          "Failed to move insert operation from child block into parent block "
          "(key=\"%s\")",
          key);
      LCH_ListDestroy(keys);
      return false;
    }
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool MergeDeleteOperations(const LCH_Json *const parent,
                                  LCH_Json *const child_deletes) {
  assert(parent != NULL);
  assert(child_deletes != NULL);

  const LCH_Json *const parent_inserts =
      LCH_JsonObjectGetObject(parent, "inserts");
  assert(parent_inserts != NULL);

  const LCH_Json *const parent_deletes =
      LCH_JsonObjectGetObject(parent, "deletes");
  assert(parent_deletes != NULL);

  const LCH_Json *const parent_updates =
      LCH_JsonObjectGetObject(parent, "updates");
  assert(parent_updates != NULL);

  LCH_List *const keys = LCH_JsonObjectGetKeys(child_deletes);
  if (keys == NULL) {
    LCH_LOG_ERROR("Failed to get keys from child block's delete operations");
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (const char *)LCH_ListGet(keys, i);

    if (LCH_JsonObjectHasKey(parent_inserts, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with insert operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_Json *const parent_value = LCH_JsonObjectRemove(parent_inserts, key);
      LCH_Json *const child_value = LCH_JsonObjectRemove(child_deletes, key);

      const bool is_equal = LCH_JsonIsEqual(parent_value, child_value);
      const bool is_null = LCH_JsonIsNull(child_value);

      LCH_JsonDestroy(parent_value);
      LCH_JsonDestroy(child_value);

      if (is_equal) {
        LCH_LOG_DEBUG(
            "Merging: insert(key, val) -> delete(key, val) => NOOP "
            "(key=\"%s\")",
            key);
      } else if (!is_null) {
        LCH_LOG_ERROR(
            "Found insert operation in parent block followed by delete "
            "operation in child block with the same key, but different values "
            "(key=\"%s\")",
            key);
        LCH_ListDestroy(keys);
        return false;
      } else {
        LCH_LOG_DEBUG(
            "Merging: insert(key, val) -> delete(key, null) => NOOP "
            "(key=\"%s\")",
            key);
      }
      continue;
    }

    if (LCH_JsonObjectHasKey(parent_deletes, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with delete operations in parent
      //////////////////////////////////////////////////////////////////////////

      // parent_delete(key, val) -> child_delete(key, val) => ERROR
      LCH_LOG_ERROR(
          "Found two subsequent delete operations with the same key in parent "
          "and child block (key='%s')",
          key);
      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_updates, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with update operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_Json *const parent_value = LCH_JsonObjectRemove(parent_updates, key);
      LCH_Json *const child_value = LCH_JsonObjectRemove(child_deletes, key);

      const bool is_equal = LCH_JsonIsEqual(parent_value, child_value);
      const bool is_null = LCH_JsonIsNull(child_value);

      LCH_JsonDestroy(parent_value);
      LCH_JsonDestroy(child_value);

      if (is_equal) {
        LCH_LOG_DEBUG(
            "Merging: update(key, val) -> delete(key, val) => delete(key, "
            "null) "
            "(key=\"%s\")",
            key);
      } else if (!is_null) {
        LCH_LOG_ERROR(
            "Found update operation in parent block followed by delete "
            "operation in child block with the same key, but different values "
            "(key=\"%s\")",
            key);
        LCH_ListDestroy(keys);
        return false;
      } else {
        LCH_LOG_DEBUG(
            "Merging: update(key, val) -> delete(key, null) => delete(key, "
            "null) "
            "(key=\"%s\")",
            key);
      }

      /* We have to use null as place holder, because we have no clue what the
       * value was before the update in the child block. This null value will be
       * carried down the chain and will end up in the final patch. However it
       * does not matter, as only the key is required in a delete operation. */
      LCH_Json *const null = LCH_JsonNullCreate();
      if (null == NULL) {
        LCH_LOG_ERROR(
            "Failed to create null value for delete operation in parent block"
            "(key=\"%s\")",
            key);
        LCH_ListDestroy(keys);
        return false;
      }

      if (!LCH_JsonObjectSet(parent_deletes, key, null)) {
        LCH_LOG_ERROR(
            "Failed to set created delete operation in parent block "
            "(key=\"%s\")",
            key);
        LCH_ListDestroy(keys);
        return false;
      }

      continue;
    }

    LCH_LOG_DEBUG(
        "Merging: NOOP -> delete(key, val) => delete(key, val) (key=\"%s\")",
        key);
    LCH_Json *const child_value = LCH_JsonObjectRemove(child_deletes, key);
    if (!LCH_JsonObjectSet(parent_deletes, key, child_value)) {
      LCH_LOG_ERROR(
          "Failed to move delete operation from child block into parent block "
          "(key=\"%s\")",
          key);
      LCH_ListDestroy(keys);
      return false;
    }
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool MergeUpdateOperations(const LCH_Json *const parent,
                                  LCH_Json *const child_updates) {
  assert(parent != NULL);
  assert(child_updates != NULL);

  const LCH_Json *const parent_inserts =
      LCH_JsonObjectGetObject(parent, "inserts");
  assert(parent_inserts != NULL);

  const LCH_Json *const parent_deletes =
      LCH_JsonObjectGetObject(parent, "deletes");
  assert(parent_deletes != NULL);

  const LCH_Json *const parent_updates =
      LCH_JsonObjectGetObject(parent, "updates");
  assert(parent_updates != NULL);

  LCH_List *const keys = LCH_JsonObjectGetKeys(child_updates);
  if (keys == NULL) {
    LCH_LOG_ERROR("Failed to get keys from child block's update operations");
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (const char *)LCH_ListGet(keys, i);

    if (LCH_JsonObjectHasKey(parent_inserts, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with insert operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_LOG_DEBUG(
          "Merging: insert(key, val1) -> update(key, val2) => insert(key, "
          "val2) (key=\"%s\")",
          key);

      LCH_Json *const child_value = LCH_JsonObjectRemove(child_updates, key);
      if (!LCH_JsonObjectSet(parent_inserts, key, child_value)) {
        LCH_LOG_ERROR(
            "Failed to move update operation from child block into insert "
            "operations in parent block (key=\"%s\")",
            key);
        LCH_ListDestroy(keys);
        return false;
      }
      continue;
    }

    if (LCH_JsonObjectHasKey(parent_deletes, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with delete operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_LOG_DEBUG(
          "Found a delete operation in parent block followed by an update "
          "operation in child block (key=\"%s\")",
          key);

      LCH_ListDestroy(keys);
      return false;
    }

    if (LCH_JsonObjectHasKey(parent_updates, key)) {
      //////////////////////////////////////////////////////////////////////////
      // Merge with update operations in parent
      //////////////////////////////////////////////////////////////////////////

      LCH_LOG_DEBUG(
          "Merging: update(key, val1) -> update(key, val2) => update(key, "
          "val2) (key=\"%s\")",
          key);

      LCH_Json *const child_value = LCH_JsonObjectRemove(child_updates, key);
      if (!LCH_JsonObjectSet(parent_updates, key, child_value)) {
        LCH_LOG_ERROR(
            "Failed to move update operation from child block into update "
            "operations in parent block (key=\"%s\")",
            key);
        LCH_JsonDestroy(child_value);
        LCH_ListDestroy(keys);
        return false;
      }
      continue;
    }

    LCH_LOG_DEBUG(
        "Merging: NOOP -> update(key, val) => update(key, val) (key=\"%s\")",
        key);
    LCH_Json *const child_value = LCH_JsonObjectRemove(child_updates, key);
    if (!LCH_JsonObjectSet(parent_updates, key, child_value)) {
      LCH_LOG_ERROR(
          "Failed to move update operation from child block into parent block "
          "(key=\"%s\")",
          key);
      LCH_ListDestroy(keys);
      return false;
    }
  }

  LCH_ListDestroy(keys);
  return true;
}

bool LCH_DeltaMerge(const LCH_Json *const parent, LCH_Json *const child) {
  assert(parent != NULL);
  assert(child != NULL);

  // We assume deltas are for the same table
  const char *const table_id = LCH_DeltaGetTableID(child);
  assert(LCH_StringEqual(table_id, LCH_DeltaGetTableID(parent)));
  LCH_LOG_VERBOSE("Merging deltas for table '%s'", table_id);

  static const char *const operations[] = {"inserts", "deletes", "updates"};

  // Make sure child block's delta has the required keys
  for (size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); i++) {
    const char *const operation = operations[i];
    if (!LCH_JsonObjectHasKey(parent, operation)) {
      LCH_LOG_ERROR(
          "Missing required key \"%s\" in parent block's delta for table '%s'",
          operation, table_id);
      return false;
    }
  }

  LCH_Json *const child_inserts = LCH_JsonObjectRemoveObject(child, "inserts");
  LCH_Json *const child_deletes = LCH_JsonObjectRemoveObject(child, "deletes");
  LCH_Json *const child_updates = LCH_JsonObjectRemoveObject(child, "updates");
  LCH_JsonDestroy(child);

  // Make sure parent block's delta has the required keys
  for (size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); i++) {
    const char *const operation = operations[i];
    if (!LCH_JsonObjectHasKey(parent, operation)) {
      LCH_LOG_ERROR(
          "Missing required key \"%s\" in parent block's delta for table '%s'",
          operation, table_id);
      LCH_JsonDestroy(child_inserts);
      LCH_JsonDestroy(child_deletes);
      LCH_JsonDestroy(child_updates);
      return false;
    }
  }

  if (!MergeInsertOperations(parent, child_inserts)) {
    LCH_LOG_ERROR(
        "Failed to merge insert operations from child into parent block's "
        "delta for table '%s'",
        table_id);
    LCH_JsonDestroy(child_inserts);
    LCH_JsonDestroy(child_deletes);
    LCH_JsonDestroy(child_updates);
    return false;
  }
  LCH_JsonDestroy(child_inserts);

  if (!MergeDeleteOperations(parent, child_deletes)) {
    LCH_LOG_ERROR(
        "Failed to merge delete operations from child into parent block's "
        "delta for table '%s'",
        table_id);
    LCH_JsonDestroy(child_deletes);
    LCH_JsonDestroy(child_updates);
    return false;
  }
  LCH_JsonDestroy(child_deletes);

  if (!MergeUpdateOperations(parent, child_updates)) {
    LCH_LOG_ERROR(
        "Failed to merge update operations from child into parent block's "
        "delta for table '%s'",
        table_id);
    LCH_JsonDestroy(child_updates);
    return false;
  }
  LCH_JsonDestroy(child_updates);

  return parent;
}
