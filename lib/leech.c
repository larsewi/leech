#include "leech.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "block.h"
#include "csv.h"
#include "definitions.h"
#include "delta.h"
#include "files.h"
#include "head.h"
#include "instance.h"
#include "json.h"
#include "logger.h"
#include "patch.h"
#include "string_lib.h"
#include "table.h"

static bool CollectGarbage(const LCH_Instance *const instance) {
  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  const size_t max_chain_length = LCH_InstanceGetMaxChainLength(instance);

  char *block_id = LCH_HeadGet("HEAD", work_dir);
  if (block_id == NULL) {
    free(block_id);
    return false;
  }

  // Traverse all the blocks that we want to keep
  char path[PATH_MAX];
  for (size_t i = 0; i < max_chain_length; i++) {
    if (!LCH_FilePathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
      return false;
    }

    if (!LCH_FileExists(path)) {
      LCH_LOG_DEBUG(
          "Block with identifier %s does not exist: "
          "End-of-Chain reached at index %zu",
          block_id, i);
      LCH_LOG_VERBOSE("Garbage collector deleted 0 blocks", i);
      free(block_id);
      return true;
    }

    LCH_Json *const block = LCH_BlockLoad(work_dir, block_id);
    free(block_id);
    if (block == NULL) {
      return false;
    }

    const char *const parent_id = LCH_BlockGetParentId(block);
    if (parent_id == NULL) {
      LCH_JsonDestroy(block);
      return false;
    }

    block_id = LCH_StringDuplicate(parent_id);
    LCH_JsonDestroy(block);
    if (block_id == NULL) {
      return false;
    }
  }

  // Now start deleting blocks

  if (!LCH_FilePathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
    return NULL;
  }

  size_t i = 0;
  while (LCH_FileExists(path)) {
    LCH_Json *const block = LCH_BlockLoad(work_dir, block_id);
    if (block == NULL) {
      free(block_id);
      return false;
    }

    LCH_LOG_DEBUG("Deleting block with identifier %.7s (path='%s')", block_id,
                  path);
    free(block_id);
    if (!LCH_FileDelete(path)) {
      LCH_JsonDestroy(block);
      return false;
    }

    const char *const parent_id = LCH_BlockGetParentId(block);
    if (parent_id == NULL) {
      LCH_JsonDestroy(block);
      return false;
    }

    block_id = LCH_StringDuplicate(parent_id);
    LCH_JsonDestroy(block);
    if (block_id == NULL) {
      return false;
    }

    if (!LCH_FilePathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
      free(block_id);
      return false;
    }

    i += 1;
  }

  free(block_id);
  LCH_LOG_VERBOSE("Garbage collector deleted %zu block(s)", i);
  return true;
}

static bool Commit(const LCH_Instance *const instance) {
  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  const bool pretty_print = LCH_InstancePrettyPrint(instance);
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

    LCH_Json *const delta =
        LCH_DeltaCreate(table_id, "delta", new_state, old_state);
    LCH_JsonDestroy(old_state);
    if (delta == NULL) {
      LCH_LOG_ERROR("Failed to compute delta for table '%s'.", table_id);
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      return false;
    }

    size_t num_inserts, num_deletes, num_updates;
    if (!LCH_DeltaGetNumOperations(delta, &num_inserts, &num_deletes,
                                   &num_updates)) {
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      return false;
    }

    LCH_LOG_VERBOSE(
        "Computed delta for table '%s' including; %zu insertions, %zu "
        "deletions, and %zu updates.",
        table_id, num_inserts, num_deletes, num_updates);
    tot_inserts += num_inserts;
    tot_deletes += num_deletes;
    tot_updates += num_updates;

    if (!LCH_JsonArrayAppend(deltas, delta)) {
      LCH_JsonDestroy(delta);
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      return false;
    }

    /************************************************************************/

    if (num_inserts > 0 || num_deletes > 0 || num_updates > 0) {
      if (!LCH_TableStoreNewState(table_def, work_dir, pretty_print,
                                  new_state)) {
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

  char *const parent_id = LCH_HeadGet("HEAD", work_dir);
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

  if (!LCH_BlockStore(instance, block)) {
    LCH_LOG_ERROR("Failed to store block.");
    LCH_JsonDestroy(block);
    return false;
  }
  LCH_JsonDestroy(block);

  LCH_LOG_INFO(
      "Created commit with %zu inserts, %zu deletes and %zu updates over %zu "
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

  if (!Commit(instance)) {
    LCH_LOG_ERROR("Failed to commit state changes");
    LCH_InstanceDestroy(instance);
    return false;
  }

  if (!CollectGarbage(instance)) {
    LCH_LOG_ERROR(
        "Failed to collect garbage: "
        "NB. there may be unreachable blocks");
    LCH_InstanceDestroy(instance);
    return false;
  }

  LCH_InstanceDestroy(instance);
  return true;
}

static LCH_Json *CreateEmptyBlock(const char *const parent_id) {
  LCH_Json *const empty_payload = LCH_JsonArrayCreate();
  if (empty_payload == NULL) {
    return NULL;
  }

  LCH_Json *block = LCH_BlockCreate(parent_id, empty_payload);
  if (block == NULL) {
    LCH_JsonDestroy(empty_payload);
    return NULL;
  }

  return block;
}

static LCH_Json *MergeBlocks(const LCH_Instance *const instance,
                             const char *const final_id,
                             LCH_Json *const child) {
  assert(instance != NULL);

  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  const char *const parent_id = LCH_BlockGetParentId(child);

  if (LCH_StringEqual(parent_id, final_id)) {
    // Base case reached. Recursion ends here.
    return child;
  }

  LCH_Json *const parent = LCH_BlockLoad(work_dir, parent_id);
  if (parent == NULL) {
    LCH_LOG_ERROR("Failed to load block with identifier %.7s", parent_id);
    LCH_JsonDestroy(child);
    return NULL;
  }
  LCH_LOG_VERBOSE("Loaded block with identifier %.7s", parent_id);

  const LCH_Json *const parent_payload = LCH_BlockGetPayload(parent);
  if (parent_payload == NULL) {
    LCH_JsonDestroy(child);
    LCH_JsonDestroy(parent);
    return NULL;
  }

  LCH_Json *const child_payload = LCH_BlockRemovePayload(child);
  LCH_JsonDestroy(child);  // We don't need the child block anymore
  if (child_payload == NULL) {
    LCH_JsonDestroy(parent);
    return NULL;
  }

  LCH_Buffer *const key = LCH_BufferFromString("id");
  if (key == NULL) {
    LCH_JsonDestroy(child_payload);
    LCH_JsonDestroy(parent);
    return NULL;
  }

  const size_t num_parent_deltas = LCH_JsonArrayLength(parent_payload);
  while (LCH_JsonArrayLength(child_payload) > 0) {
    LCH_Json *const child_delta = LCH_JsonArrayRemoveObject(child_payload, 0);
    if (child_delta == NULL) {
      LCH_BufferDestroy(key);
      LCH_JsonDestroy(child_payload);
      LCH_JsonDestroy(parent);
      return NULL;
    }

    const LCH_Buffer *const child_table_id =
        LCH_JsonObjectGetString(child_delta, key);
    if (child_table_id == NULL) {
      LCH_JsonDestroy(child_delta);
      LCH_BufferDestroy(key);
      LCH_JsonDestroy(child_payload);
      LCH_JsonDestroy(parent);
      return NULL;
    }

    bool found = false;
    const LCH_Json *parent_delta = NULL;

    for (size_t i = 0; i < num_parent_deltas; i++) {
      parent_delta = LCH_JsonArrayGetObject(parent_payload, i);
      if (parent_delta == NULL) {
        LCH_JsonDestroy(child_delta);
        LCH_BufferDestroy(key);
        LCH_JsonDestroy(child_payload);
        LCH_JsonDestroy(parent);
        return NULL;
      }

      const LCH_Buffer *const parent_table_id =
          LCH_JsonObjectGetString(parent_delta, key);
      if (parent_table_id == NULL) {
        LCH_JsonDestroy(child_delta);
        LCH_BufferDestroy(key);
        LCH_JsonDestroy(child_payload);
        LCH_JsonDestroy(parent);
        return NULL;
      }

      if (LCH_BufferEqual(parent_table_id, child_table_id)) {
        found = true;
        break;
      }
    }

    if (found) {
      if (!LCH_DeltaMerge(parent_delta, child_delta)) {
        LCH_LOG_ERROR(
            "Failed to merge parent block delta with child block delta for "
            "table '%s'",
            child_table_id);
        LCH_JsonDestroy(child_delta);
        LCH_BufferDestroy(key);
        LCH_JsonDestroy(child_payload);
        LCH_JsonDestroy(parent);
        return NULL;
      }
    } else {
      if (!LCH_JsonArrayAppend(parent, child_delta)) {
        LCH_LOG_ERROR(
            "Failed to append child block delta for table '%s' to parent block "
            "payload",
            child_table_id);
        LCH_JsonDestroy(child_delta);
        LCH_BufferDestroy(key);
        LCH_JsonDestroy(child_payload);
        LCH_JsonDestroy(parent);
        return NULL;
      }
    }
    LCH_JsonDestroy(child_delta);
  }

  LCH_BufferDestroy(key);
  LCH_JsonDestroy(child_payload);
  LCH_Json *const merged = MergeBlocks(instance, final_id, parent);
  return merged;
}

LCH_Buffer *LCH_Diff(const char *const work_dir, const char *const final_id) {
  assert(work_dir != NULL);
  assert(final_id != NULL);

  LCH_Instance *const instance = LCH_InstanceLoad(work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to load instance from configuration file");
    return NULL;
  }

  const bool pretty_print = LCH_InstancePrettyPrint(instance);

  char *const block_id = LCH_HeadGet("HEAD", work_dir);
  if (block_id == NULL) {
    LCH_LOG_ERROR(
        "Failed to get block identifier from the head of the chain. "
        "Maybe there has not been any commits yet?");
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_Json *const patch = LCH_PatchCreate(block_id);
  if (patch == NULL) {
    LCH_LOG_ERROR("Failed to create patch");
    free(block_id);
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_Json *const empty = CreateEmptyBlock(block_id);
  free(block_id);
  if (empty == NULL) {
    LCH_LOG_ERROR("Failed to create empty block");
    LCH_JsonDestroy(patch);
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_Json *const block = MergeBlocks(instance, final_id, empty);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to generate patch file");
    LCH_JsonDestroy(patch);
    LCH_InstanceDestroy(instance);
    return NULL;
  }
  LCH_InstanceDestroy(instance);

  if (!LCH_PatchAppendBlock(patch, block)) {
    LCH_LOG_ERROR("Failed to append block to patch");
    LCH_JsonDestroy(block);
    LCH_JsonDestroy(patch);
    return NULL;
  }

  LCH_Buffer *buffer = LCH_JsonCompose(patch, pretty_print);
  LCH_JsonDestroy(patch);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to compose patch into JSON");
    return NULL;
  }
  return buffer;
}

LCH_Buffer *LCH_Rebase(const char *const work_dir) {
  LCH_Instance *const instance = LCH_InstanceLoad(work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to load instance from configuration file");
    return NULL;
  }

  const bool pretty_print = LCH_InstancePrettyPrint(instance);

  const LCH_List *const table_defs = LCH_InstanceGetTables(instance);
  size_t n_tables = LCH_ListLength(table_defs);
  size_t tot_inserts;

  LCH_Json *const deltas = LCH_JsonArrayCreate();
  if (deltas == NULL) {
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  for (size_t i = 0; i < n_tables; i++) {
    const LCH_TableInfo *const table_def =
        (LCH_TableInfo *)LCH_ListGet(table_defs, i);
    const char *const table_id = LCH_TableInfoGetIdentifier(table_def);

    /************************************************************************/
    LCH_Json *const new_state = LCH_TableInfoLoadOldState(table_def, work_dir);
    if (new_state == NULL) {
      LCH_LOG_ERROR("Failed to load old state as new state for table '%s'.",
                    table_id);
      LCH_JsonDestroy(deltas);
      LCH_InstanceDestroy(instance);
      return NULL;
    }
    LCH_LOG_VERBOSE(
        "Loaded old state as new state for table '%s' containing %zu rows.",
        table_id, LCH_JsonObjectLength(new_state));

    LCH_Json *const old_state = LCH_JsonObjectCreate();
    if (old_state == NULL) {
      LCH_LOG_ERROR("Failed create fake empty old state for table '%s'.",
                    table_id);
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      LCH_InstanceDestroy(instance);
      return NULL;
    }
    LCH_LOG_VERBOSE(
        "Created fake empty old state for table '%s' containing %zu rows.",
        table_id, LCH_JsonObjectLength(old_state));

    /************************************************************************/

    LCH_Json *const delta =
        LCH_DeltaCreate(table_id, "rebase", new_state, old_state);
    LCH_JsonDestroy(old_state);
    LCH_JsonDestroy(new_state);
    if (delta == NULL) {
      LCH_LOG_ERROR("Failed to compute rebase delta for table '%s'.", table_id);
      LCH_JsonDestroy(deltas);
      LCH_InstanceDestroy(instance);
      return NULL;
    }

    size_t num_inserts;
    if (!LCH_DeltaGetNumOperations(delta, &num_inserts, NULL, NULL)) {
      LCH_JsonDestroy(deltas);
      LCH_InstanceDestroy(instance);
      return NULL;
    }

    LCH_LOG_VERBOSE(
        "Computed rebase delta for table '%s' including; %zu insertions",
        table_id, num_inserts);
    tot_inserts += num_inserts;

    if (!LCH_JsonArrayAppend(deltas, delta)) {
      LCH_JsonDestroy(delta);
      LCH_JsonDestroy(deltas);
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  char *const parent_id = LCH_HeadGet("HEAD", work_dir);
  if (parent_id == NULL) {
    LCH_LOG_ERROR("Failed to get identifier for block at head of chain");
    LCH_JsonDestroy(deltas);
    return NULL;
  }

  LCH_InstanceDestroy(instance);

  LCH_Json *const block = LCH_BlockCreate(parent_id, deltas);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to create block.");
    free(parent_id);
    LCH_JsonDestroy(deltas);
    return NULL;
  }

  LCH_Json *const patch = LCH_PatchCreate(parent_id);
  free(parent_id);
  if (patch == NULL) {
    LCH_LOG_ERROR("Failed to create patch");
    return NULL;
  }

  if (!LCH_PatchAppendBlock(patch, block)) {
    LCH_LOG_ERROR("Failed to append block to patch");
    LCH_JsonDestroy(block);
    LCH_JsonDestroy(patch);
    return NULL;
  }

  LCH_Buffer *const buffer = LCH_JsonCompose(patch, pretty_print);
  LCH_JsonDestroy(patch);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to compose patch into JSON");
    return NULL;
  }

  return buffer;
}

static bool HistoryAppendRecord(const LCH_Json *const history,
                                const char *const block_id,
                                const double timestamp,
                                const char *const operation,
                                const LCH_Buffer *const subsidiary_value) {
  LCH_Json *const record = LCH_JsonObjectCreate();
  if (record == NULL) {
    return false;
  }

  {
    LCH_Buffer *const value = LCH_BufferFromString(block_id);
    if (value == NULL) {
      LCH_JsonDestroy(record);
      return false;
    }

    const LCH_Buffer *const key = LCH_BufferStaticFromString("block_id");
    if (!LCH_JsonObjectSetString(record, key, value)) {
      LCH_BufferDestroy(value);
      LCH_JsonDestroy(record);
      return false;
    }
  }

  {
    const LCH_Buffer *const key = LCH_BufferStaticFromString("timestamp");
    if (!LCH_JsonObjectSetNumber(record, key, timestamp)) {
      LCH_JsonDestroy(record);
      return false;
    }
  }

  {
    LCH_Buffer *const value = LCH_BufferFromString(operation);
    if (value == NULL) {
      LCH_JsonDestroy(record);
      return false;
    }

    const LCH_Buffer *const key = LCH_BufferStaticFromString("operation");
    if (!LCH_JsonObjectSetString(record, key, value)) {
      LCH_BufferDestroy(value);
      LCH_JsonDestroy(record);
      return false;
    }
  }

  {
    const char *const data = LCH_BufferData(subsidiary_value);
    const size_t length = LCH_BufferLength(subsidiary_value);

    LCH_List *const subsidiary_fields = LCH_CSVParseRecord(data, length);
    if (subsidiary_fields == NULL) {
      LCH_JsonDestroy(record);
      return false;
    }

    LCH_Json *const value = LCH_JsonArrayCreate();
    if (value == NULL) {
      LCH_ListDestroy(subsidiary_fields);
      LCH_JsonDestroy(record);
      return false;
    }

    const size_t num_fields = LCH_ListLength(subsidiary_fields);
    for (size_t i = 0; i < num_fields; i++) {
      const LCH_Buffer *const field =
          (LCH_Buffer *)LCH_ListGet(subsidiary_fields, i);
      if (!LCH_JsonArrayAppendStringDuplicate(value, field)) {
        LCH_JsonDestroy(value);
        LCH_ListDestroy(subsidiary_fields);
        LCH_JsonDestroy(record);
        return false;
      }
    }
    LCH_ListDestroy(subsidiary_fields);

    const LCH_Buffer *const key = LCH_BufferStaticFromString("subsidiary");
    if (!LCH_JsonObjectSet(record, key, value)) {
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(record);
      return false;
    }
  }

  if (!LCH_JsonArrayAppend(history, record)) {
    LCH_JsonDestroy(record);
    return false;
  }

  LCH_Buffer *const str_repr = LCH_JsonCompose(record, true);
  if (str_repr != NULL) {
    LCH_LOG_DEBUG("Found entry: %s", LCH_BufferData(str_repr));
    LCH_BufferDestroy(str_repr);
  }

  return true;
}

static bool HistoryFindRecord(const LCH_Instance *const instance,
                              const LCH_Json *const history,
                              const LCH_Buffer *const primary_key,
                              const char *const block_id, const double from,
                              const double to) {
  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);

  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
    return false;
  }

  if (!LCH_FileExists(path)) {
    LCH_LOG_VERBOSE("Reached End-of-Chain with block identifier '%s'",
                    block_id);
    return true;
  }

  LCH_Json *const block = LCH_BlockLoad(work_dir, block_id);
  if (block == NULL) {
    return false;
  }

  const char *const parent_id = LCH_BlockGetParentId(block);
  if (parent_id == NULL) {
    LCH_JsonDestroy(block);
    return false;
  }

  double timestamp;
  if (!LCH_BlockGetTimestamp(block, &timestamp)) {
    LCH_JsonDestroy(block);
    return false;
  }

  if (timestamp < from) {
    // Base case reached, stop recording history
    LCH_JsonDestroy(block);
    return true;
  }

  if (timestamp >= to) {
    // Continue without recording history (yet)
    if (!HistoryFindRecord(instance, history, primary_key, parent_id, from,
                           to)) {
      LCH_JsonDestroy(block);
      return false;
    }

    LCH_JsonDestroy(block);
    return true;
  }

  const LCH_Json *const payload = LCH_BlockGetPayload(block);
  if (payload == NULL) {
    LCH_JsonDestroy(block);
    return false;
  }

  const size_t num_deltas = LCH_JsonArrayLength(payload);
  for (size_t i = 0; i < num_deltas; i++) {
    const LCH_Json *const delta = LCH_JsonArrayGetObject(payload, i);
    if (delta == NULL) {
      LCH_JsonDestroy(block);
      return false;
    }

    const LCH_Json *const inserts = LCH_DeltaGetInserts(delta);
    if (inserts == NULL) {
      LCH_JsonDestroy(block);
      return false;
    }

    const LCH_Json *const updates = LCH_DeltaGetUpdates(delta);
    if (updates == NULL) {
      LCH_JsonDestroy(block);
      return false;
    }

    const LCH_Json *const deletes = LCH_DeltaGetDeletes(delta);
    if (deletes == NULL) {
      LCH_JsonDestroy(block);
      return false;
    }

    if (LCH_JsonObjectHasKey(inserts, primary_key)) {
      const LCH_Buffer *const subsidiary_value =
          LCH_JsonObjectGetString(inserts, primary_key);
      if (subsidiary_value == NULL) {
        LCH_JsonDestroy(block);
        return false;
      }

      if (!HistoryAppendRecord(history, block_id, timestamp, "insert",
                               subsidiary_value)) {
        LCH_JsonDestroy(block);
        return false;
      }
    } else if (LCH_JsonObjectHasKey(deletes, primary_key)) {
      const LCH_Buffer *const subsidiary_value =
          LCH_JsonObjectGetString(deletes, primary_key);
      if (subsidiary_value == NULL) {
        LCH_JsonDestroy(block);
        return false;
      }

      if (!HistoryAppendRecord(history, block_id, timestamp, "delete",
                               subsidiary_value)) {
        LCH_JsonDestroy(block);
        return false;
      }
    } else if (LCH_JsonObjectHasKey(updates, primary_key)) {
      const LCH_Buffer *const subsidiary_value =
          LCH_JsonObjectGetString(updates, primary_key);
      if (subsidiary_value == NULL) {
        LCH_JsonDestroy(block);
        return false;
      }

      if (!HistoryAppendRecord(history, block_id, timestamp, "update",
                               subsidiary_value)) {
        LCH_JsonDestroy(block);
        return false;
      }
    }
  }

  if (!HistoryFindRecord(instance, history, primary_key, parent_id, from, to)) {
    LCH_JsonDestroy(block);
    return false;
  }

  LCH_JsonDestroy(block);
  return true;
}

LCH_Buffer *LCH_History(const char *const work_dir,
                        const LCH_List *const primary_fields, const double from,
                        const double to) {
  LCH_Instance *const instance = LCH_InstanceLoad(work_dir);
  if (instance == NULL) {
    return NULL;
  }

  LCH_Json *const response = LCH_JsonObjectCreate();
  if (response == NULL) {
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_Json *const history = LCH_JsonArrayCreate();
  if (history == NULL) {
    LCH_JsonDestroy(response);
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  {
    const LCH_Buffer *const key = LCH_BufferStaticFromString("history");
    if (!LCH_JsonObjectSet(response, key, history)) {
      LCH_JsonDestroy(history);
      LCH_JsonDestroy(response);
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  {
    LCH_Json *const primary = LCH_JsonArrayCreate();
    if (primary == NULL) {
      LCH_JsonDestroy(response);
      LCH_InstanceDestroy(instance);
      return NULL;
    }

    const size_t num_fields = LCH_ListLength(primary_fields);
    for (size_t i = 0; i < num_fields; i++) {
      const LCH_Buffer *const field =
          (LCH_Buffer *)LCH_ListGet(primary_fields, i);
      if (!LCH_JsonArrayAppendStringDuplicate(primary, field)) {
        LCH_JsonDestroy(primary);
        LCH_JsonDestroy(response);
        LCH_InstanceDestroy(instance);
        return NULL;
      }
    }

    const LCH_Buffer *const key = LCH_BufferStaticFromString("primary");
    if (!LCH_JsonObjectSet(response, key, primary)) {
      LCH_JsonDestroy(primary);
      LCH_JsonDestroy(response);
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  {
    const LCH_Buffer *const key = LCH_BufferStaticFromString("from");
    if (!LCH_JsonObjectSetNumber(response, key, from)) {
      LCH_JsonDestroy(response);
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  {
    const LCH_Buffer *const key = LCH_BufferStaticFromString("to");
    if (!LCH_JsonObjectSetNumber(response, key, to)) {
      LCH_JsonDestroy(response);
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  char *block_id = LCH_HeadGet("HEAD", work_dir);
  if (block_id == NULL) {
    LCH_JsonDestroy(response);
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_Buffer *primary = NULL;
  if (!LCH_CSVComposeRecord(&primary, primary_fields)) {
    LCH_JsonDestroy(response);
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  if (!HistoryFindRecord(instance, history, primary, block_id, from, to)) {
    LCH_BufferDestroy(primary);
    free(block_id);
    LCH_BufferDestroy(response);
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_BufferDestroy(primary);
  free(block_id);

  const bool pretty = LCH_InstancePrettyPrint(instance);
  LCH_Buffer *const buffer = LCH_JsonCompose(response, pretty);
  LCH_JsonDestroy(response);
  if (buffer == NULL) {
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  LCH_InstanceDestroy(instance);
  return buffer;
}

static bool Patch(const LCH_Instance *const instance, const char *const field,
                  const char *const value, const char *const buffer,
                  const size_t size) {
  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);

  LCH_Json *const patch = LCH_JsonParse(buffer, size);
  if (patch == NULL) {
    LCH_LOG_ERROR("Failed to parse patch");
    return false;
  }

  if (!LCH_PatchUpdateLastKnown(patch, work_dir, value)) {
    LCH_LOG_ERROR("Failed to update lastseen");
    LCH_JsonDestroy(patch);
    return false;
  }

  const LCH_Json *const blocks =
      LCH_JsonObjectGetArray(patch, LCH_BufferStaticFromString("blocks"));
  if (blocks == NULL) {
    LCH_LOG_ERROR("Failed to extract blocks from patch");
    LCH_JsonDestroy(patch);
    return false;
  }

  const size_t num_blocks = LCH_JsonArrayLength(blocks);
  for (size_t i = 0; i < num_blocks; i++) {
    LCH_LOG_DEBUG("Extracting block at index %zu", i);

    const LCH_Json *const block = LCH_JsonArrayGetObject(blocks, i);
    if (block == NULL) {
      LCH_LOG_ERROR("Failed to extract block");
      LCH_JsonDestroy(patch);
      return false;
    }

    const LCH_Json *const payload =
        LCH_JsonObjectGetArray(block, LCH_BufferStaticFromString("payload"));
    if (payload == NULL) {
      LCH_LOG_ERROR("Failed to extract payload");
      LCH_JsonDestroy(patch);
      return false;
    }

    const size_t num_deltas = LCH_JsonArrayLength(payload);
    for (size_t j = 0; j < num_deltas; j++) {
      LCH_LOG_DEBUG("Extracting delta at index %zu", j);

      const LCH_Json *const delta = LCH_JsonArrayGetObject(payload, j);
      if (delta == NULL) {
        LCH_LOG_ERROR("Failed to extract delta");
        LCH_JsonDestroy(patch);
        return false;
      }

      const LCH_Buffer *type_buf =
          LCH_JsonObjectGetString(delta, LCH_BufferStaticFromString("type"));
      if (type_buf == NULL) {
        LCH_LOG_ERROR("Failed to extract type from delta");
        LCH_JsonDestroy(patch);
        return false;
      }
      const char *const type = LCH_BufferData(type_buf);

      const LCH_Buffer *const table_id_buf =
          LCH_JsonObjectGetString(delta, LCH_BufferStaticFromString("id"));
      if (table_id_buf == NULL) {
        LCH_LOG_ERROR("Failed to extract table ID from delta");
        LCH_JsonDestroy(patch);
        return false;
      }
      const char *const table_id = LCH_BufferData(table_id_buf);

      const LCH_TableInfo *const table_info =
          LCH_InstanceGetTable(instance, table_id);
      if (table_info == NULL) {
        LCH_LOG_WARNING(
            "Table with identifer '%s' not found in config file. Skipping "
            "patch...",
            table_id);
        continue;
      }

      const LCH_Json *const inserts = LCH_DeltaGetInserts(delta);
      if (inserts == NULL) {
        LCH_JsonDestroy(patch);
        return false;
      }

      const LCH_Json *const deletes = LCH_DeltaGetDeletes(delta);
      if (deletes == NULL) {
        LCH_JsonDestroy(patch);
        return false;
      }

      const LCH_Json *const updates = LCH_DeltaGetUpdates(delta);
      if (updates == NULL) {
        LCH_JsonDestroy(patch);
        return false;
      }

      if (!LCH_TablePatch(table_info, type, field, value, inserts, deletes,
                          updates)) {
        LCH_JsonDestroy(patch);
        return false;
      }
    }
  }

  LCH_JsonDestroy(patch);
  return true;
}

bool LCH_Patch(const char *const work_dir, const char *const field,
               const char *const value, const char *const patch,
               const size_t size) {
  LCH_Instance *const instance = LCH_InstanceLoad(work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to load instance from configuration file");
    return false;
  }

  const bool success = Patch(instance, field, value, patch, size);
  LCH_InstanceDestroy(instance);
  if (!success) {
    LCH_LOG_ERROR("Failed to apply patch");
  }
  return success;
}
