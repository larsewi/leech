#include "leech.h"

#include <assert.h>
#include <errno.h>
#include <openssl/sha.h>
#include <string.h>

#include "block.h"
#include "delta.h"
#include "head.h"
#include "instance.h"
#include "json.h"
#include "table.h"
#include "utils.h"

static bool Commit(const LCH_Instance *const instance) {
  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  const LCH_List *const table_defs = LCH_InstanceGetTables(instance);
  size_t n_tables = LCH_ListLength(table_defs);
  size_t tot_inserts = 0, tot_deletes = 0, tot_updates = 0;

  LCH_Json *const deltas = LCH_JsonArrayCreate();
  if (deltas == NULL) {
    return false;
  }

  for (size_t i = 0; i < n_tables; i++) {
    const LCH_TableInfo *const table_def =
        (LCH_TableInfo *)LCH_ListGet(table_defs, i);
    const char *const table_id = LCH_TableInfoGetIdentifier(table_def);

    /************************************************************************/

    LCH_Json *const new_state = LCH_TableInfoLoadNewState(table_def);
    if (new_state == NULL) {
      LCH_LOG_ERROR("Failed to load new state for table '%s'.", table_id);
      LCH_JsonDestroy(deltas);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded new state for table '%s' containing %zu rows.",
                    table_id, LCH_JsonObjectLength(new_state));

    LCH_Json *const old_state = LCH_TableInfoLoadOldState(table_def, work_dir);
    if (old_state == NULL) {
      LCH_LOG_ERROR("Failed to load old state for table '%s'.", table_id);
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded old state for table '%s' containing %zu rows.",
                    table_id, LCH_JsonObjectLength(old_state));

    /************************************************************************/

    LCH_Delta *const delta = LCH_DeltaCreate(table_id, new_state, old_state);
    LCH_JsonDestroy(old_state);
    if (delta == NULL) {
      LCH_LOG_ERROR("Failed to compute delta for table '%s'.", table_id);
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      return false;
    }

    const size_t num_inserts = LCH_DeltaGetNumInserts(delta);
    const size_t num_deletes = LCH_DeltaGetNumDeletes(delta);
    const size_t num_updates = LCH_DeltaGetNumUpdates(delta);
    LCH_LOG_VERBOSE(
        "Computed delta for table '%s' including; %zu insertions, %zu "
        "deletions, and %zu updates.",
        table_id, num_inserts, num_deletes, num_updates);
    tot_inserts += num_inserts;
    tot_deletes += num_deletes;
    tot_updates += num_updates;

    if (!LCH_JsonArrayAppend(deltas, delta)) {
      LCH_DeltaDestroy(delta);
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      return false;
    }

    /************************************************************************/

    if (num_inserts > 0 || num_deletes > 0 || num_updates > 0) {
      if (!LCH_TableStoreNewState(table_def, work_dir, new_state)) {
        LCH_LOG_ERROR("Failed to store new state for table '%s'.", table_id);
        LCH_JsonDestroy(new_state);
        LCH_JsonDestroy(deltas);
        return false;
      }
      LCH_LOG_VERBOSE("Stored new state for table '%s' containing %zu rows.",
                      table_id, LCH_JsonObjectLength(new_state));
    } else {
      LCH_LOG_DEBUG(
          "Zero changes made in table '%s'; skipping snapshot update.",
          table_id);
    }
    LCH_JsonDestroy(new_state);
  }

  char *const parent_id = LCH_HeadGet(work_dir);
  if (parent_id == NULL) {
    LCH_LOG_ERROR("Failed to get identifier for block at head of chain");
    LCH_JsonDestroy(deltas);
    return false;
  }

  LCH_Json *const block = LCH_BlockCreate(parent_id, deltas);
  free(parent_id);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to create block.");
    LCH_JsonDestroy(deltas);
    return false;
  }

  if (!LCH_BlockStore(work_dir, block)) {
    LCH_LOG_ERROR("Failed to store block.");
    LCH_JsonDestroy(block);
    return false;
  }
  LCH_JsonDestroy(block);

  LCH_LOG_VERBOSE(
      "Created commit with %zu inserts, %zu deletes and %zu inserts over %zu "
      "tables",
      tot_inserts, tot_deletes, tot_updates, n_tables);

  return true;
}

bool LCH_Commit(const char *const work_dir) {
  assert(work_dir != NULL);

  LCH_Instance *const instance = LCH_InstanceLoad(work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to load instance from configuration file");
    return false;
  }

  const bool success = Commit(instance);
  if (!success) {
    LCH_LOG_ERROR("Failed to commit");
  }
  LCH_InstanceDestroy(instance);
  return success;
}

static LCH_Block *CreateEmptyBlock(const char *const parent_id) {
  LCH_Json *const empty_payload = LCH_JsonArrayCreate();
  if (empty_payload == NULL) {
    return NULL;
  }

  LCH_Block *block = LCH_BlockCreate(parent_id, empty_payload);
  if (block == NULL) {
    LCH_JsonDestroy(empty_payload);
    return NULL;
  }

  return block;
}

static LCH_Block *MergeBlocks(const LCH_Instance *const instance,
                              const char *const final_id, LCH_Block *block) {
  assert(instance != NULL);

  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  const char *const parent_id = LCH_BlockGetParentBlockIdentifier(block);

  if (LCH_StringEqual(parent_id, final_id)) {
    // Base case reached. Recursion ends here.
    return block;
  }

  LCH_Block *const parent = LCH_BlockLoad(work_dir, parent_id);
  if (parent == NULL) {
    LCH_LOG_ERROR("Failed to load block with identifier %.7s", parent_id);
    return NULL;
  }
  LCH_LOG_VERBOSE("Loaded block with identifier %.7s", parent_id);

  // Merge delta here
  LCH_LOG_DEBUG("Merge delta here");

  LCH_BlockDestroy(block);
  block = MergeBlocks(instance, final_id, parent);
  return block;
}

char *LCH_Diff(const char *const work_dir, const char *const final_id,
               size_t *const buf_len) {
  assert(work_dir != NULL);
  assert(final_id != NULL);

  LCH_Instance *const instance = LCH_InstanceLoad(work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to load instance from configuration file");
    return NULL;
  }

  char *const block_id = LCH_HeadGet(work_dir);
  if (block_id == NULL) {
    LCH_LOG_ERROR(
        "Failed to get block identifier from the head of the chain. "
        "Maybe there has not been any commits yet?");
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_Block *block = CreateEmptyBlock(block_id);
  free(block_id);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to create empty block for patch file generation");
    return NULL;
  }

  block = MergeBlocks(instance, final_id, block);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to generate patch file");
    LCH_InstanceDestroy(instance);
    return NULL;
  }
  LCH_InstanceDestroy(instance);

  char *buffer = LCH_JsonCompose(block);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to compose patch into JSON");
    return NULL;
  }
  *buf_len = strlen(buffer);
  return buffer;
}

static bool Patch(const LCH_Instance *const instance, const char *const field,
                  const char *const value, const char *const patch,
                  const size_t size) {
  (void)instance;
  (void)field;
  (void)value;
  (void)patch;
  (void)size;

  return true;
}

bool LCH_Patch(const char *const work_dir, const char *const field,
               const char *const value, const char *const patch,
               const size_t size) {
  assert(work_dir != NULL);
  assert(field != NULL);
  assert(value != NULL);
  assert(patch != NULL);

  LCH_Instance *const instance = LCH_InstanceLoad(work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to load instance from configuration file");
    return false;
  }

  const bool success = Patch(instance, field, value, patch, size);
  if (!success) {
    LCH_LOG_ERROR("Failed to apply patch");
  }
  return true;
}
