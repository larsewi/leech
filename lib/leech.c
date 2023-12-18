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

bool LCH_Commit(const LCH_Instance *const instance) {
  assert(instance != NULL);

  const LCH_List *const table_defs = LCH_InstanceGetTables(instance);
  size_t n_tables = LCH_ListLength(table_defs);

  size_t tot_inserts = 0, tot_deletes = 0, tot_updates = 0;
  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);

  LCH_Json *const deltas = LCH_JsonArrayCreate();
  if (deltas == NULL) {
    return false;
  }

  for (size_t i = 0; i < n_tables; i++) {
    const LCH_TableDefinition *const table_def =
        (LCH_TableDefinition *)LCH_ListGet(table_defs, i);
    const char *const table_id = LCH_TableDefinitionGetIdentifier(table_def);

    /************************************************************************/

    LCH_Json *const new_state = LCH_TableDefinitionLoadNewState(table_def);
    if (new_state == NULL) {
      LCH_LOG_ERROR("Failed to load new state for table '%s'.", table_id);
      LCH_JsonDestroy(deltas);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded new state for table '%s' containing %zu rows.",
                    table_id, LCH_JsonObjectLength(new_state));

    LCH_Json *const old_state =
        LCH_TableDefinitionLoadOldState(table_def, work_dir);
    if (old_state == NULL) {
      LCH_LOG_ERROR("Failed to load old state for table '%s'.", table_id);
      LCH_JsonDestroy(new_state);
      LCH_JsonDestroy(deltas);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded old state for table '%s' containing %zu rows.",
                    table_id, LCH_JsonObjectLength(old_state));

    /************************************************************************/

    LCH_Json *const delta = LCH_DeltaCreate(table_id, new_state, old_state);
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
      LCH_JsonDestroy(delta);
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

  LCH_Json *const block = LCH_BlockCreate(work_dir, deltas);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to create block.");
    LCH_JsonDestroy(deltas);
    return false;
  }

  if (!LCH_BlockStore(block, work_dir)) {
    LCH_LOG_ERROR("Failed to store block.");
    LCH_JsonDestroy(block);
    return false;
  }
  LCH_JsonDestroy(block);

  return true;
}

char *LCH_Diff(const LCH_Instance *const instance, const char *const block_id,
               size_t *const buf_len) {
  assert(instance != NULL);
  assert(block_id != NULL);
  (void)instance;
  (void)block_id;
  (void)buf_len;

  return LCH_StringDuplicate("placeholder");
}

bool LCH_Patch(const LCH_Instance *const instance, const char *const uid_field,
               const char *const uid_value, const char *const patch,
               const size_t size) {
  assert(instance != NULL);
  assert(patch != NULL);
  (void)instance;
  (void)uid_field;
  (void)uid_value;
  (void)patch;
  (void)size;

  return true;
}
