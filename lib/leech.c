#include "leech.h"

#include <assert.h>
#include <errno.h>
#include <openssl/sha.h>
#include <string.h>

#include "block.h"
#include "delta.h"
#include "head.h"
#include "instance.h"
#include "table.h"

bool LCH_Commit(const LCH_Instance *const instance) {
  assert(instance != NULL);

  const LCH_List *const table_defs = LCH_InstanceGetTables(instance);
  size_t n_tables = LCH_ListLength(table_defs);

  LCH_Buffer *delta_buffer = LCH_BufferCreate();
  if (delta_buffer == NULL) {
    return NULL;
  }

  size_t tot_ins = 0, tot_del = 0, tot_mod = 0;
  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);

  for (size_t i = 0; i < n_tables; i++) {
    const LCH_TableDefinition *const table_def =
        (LCH_TableDefinition *)LCH_ListGet(table_defs, i);
    const char *const table_id = LCH_TableDefinitionGetIdentifier(table_def);

    /************************************************************************/

    LCH_Dict *new_state = LCH_TableDefinitionLoadNewState(table_def);
    if (new_state == NULL) {
      LCH_LOG_ERROR("Failed to load new state for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded new state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(new_state));

    LCH_Dict *old_state = LCH_TableDefinitionLoadOldState(table_def, work_dir);
    if (old_state == NULL) {
      LCH_LOG_ERROR("Failed to load old state for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(new_state);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded old state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(old_state));

    /************************************************************************/

    LCH_Delta *const delta = LCH_DeltaCreate(table_def, new_state, old_state);
    if (delta == NULL) {
      LCH_LOG_ERROR("Failed to compute delta for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(old_state);
      LCH_DictDestroy(new_state);
      return false;
    }

    const size_t num_ins = LCH_DeltaGetNumInsertions(delta);
    const size_t num_del = LCH_DeltaGetNumDeletions(delta);
    const size_t num_mod = LCH_DeltaGetNumUpdates(delta);
    LCH_LOG_VERBOSE(
        "Computed delta for table '%s' including %zu insertions, %zu "
        "deletions, and %zu modifications.",
        table_id, num_ins, num_del, num_mod);

    tot_ins += num_ins;
    tot_del += num_del;
    tot_mod += num_mod;

    if (!LCH_DeltaMarshal(delta_buffer, delta)) {
      LCH_LOG_ERROR("Failed to marshal computed delta for table '%s'.",
                    table_id);
      LCH_DeltaDestroy(delta);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(old_state);
      LCH_DictDestroy(new_state);
      return false;
    }

    LCH_DeltaDestroy(delta);
    LCH_DictDestroy(old_state);

    /************************************************************************/

    if (num_ins == 0 && num_del == 0 && num_mod == 0) {
      LCH_LOG_DEBUG("No changes made in table '%s'; skipping snapshot update.",
                    table_id);
      LCH_DictDestroy(new_state);
      continue;
    }

    if (!LCH_TableStoreNewState(table_def, work_dir, new_state)) {
      LCH_LOG_ERROR("Failed to store new state for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(new_state);
      return false;
    }
    LCH_LOG_VERBOSE("Stored new state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(new_state));
  }

  char *const head = LCH_HeadGet("HEAD", work_dir);
  if (head == NULL) {
    LCH_LOG_ERROR("Failed to get head.");
    LCH_BufferDestroy(delta_buffer);
    return false;
  }

  LCH_Block *const block = LCH_BlockCreate(head, LCH_BufferGet(delta_buffer, 0),
                                           LCH_BufferLength(delta_buffer));
  LCH_BufferDestroy(delta_buffer);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to create block.");
    free(head);
    return false;
  }

  char *const block_id = LCH_BlockStore(work_dir, block);
  if (block_id == NULL) {
    LCH_LOG_ERROR("Failed to store block.");
    free(block);
    free(head);
    return false;
  }
  free(block);
  LCH_LOG_INFO(
      "Created block '%.5s' containing %zu insertions, %zu deletions, and %zu "
      "updates, over %zu table(s).",
      block_id, tot_ins, tot_del, tot_mod, LCH_ListLength(table_defs));

  if (!LCH_HeadSet("HEAD", work_dir, block_id)) {
    LCH_LOG_ERROR("Failed to move head to '%.5s'.", block_id);
    free(head);
    free(block_id);
    return false;
  }
  LCH_LOG_VERBOSE("Moved head from '%.5s' to '%.5s'.", head, block_id);

  free(head);
  free(block_id);
  return true;
}

static LCH_Dict *CreateEmptyDeltas(const LCH_Instance *const instance) {
  assert(instance != NULL);

  LCH_Dict *const deltas = LCH_DictCreate();
  if (deltas == NULL) {
    return NULL;
  }

  const LCH_List *const table_defs = LCH_InstanceGetTables(instance);
  const size_t num_tables = LCH_ListLength(table_defs);
  for (size_t i = 0; i < num_tables; i++) {
    const LCH_TableDefinition *const table_def =
        (LCH_TableDefinition *)LCH_ListGet(table_defs, i);
    assert(table_def != NULL);

    LCH_Delta *const delta = LCH_DeltaCreate(table_def, NULL, NULL);
    if (delta == NULL) {
      LCH_DictDestroy(deltas);
      return NULL;
    }

    if (!LCH_DictSet(deltas, LCH_TableDefinitionGetIdentifier(table_def), delta,
                     LCH_DeltaDestroy)) {
      LCH_DeltaDestroy(delta);
      LCH_DictDestroy(deltas);
      return NULL;
    }
  }

  return deltas;
}

static bool CompressDeltas(LCH_Dict *const deltas,
                           const LCH_Instance *const instance,
                           const char *const buffer, const size_t buf_len,
                           size_t *const num_merged,
                           size_t *const num_canceled) {
  const char *buf_ptr = buffer;

  while ((size_t)(buf_ptr - buffer) < buf_len) {
    LCH_Delta *parent = NULL;
    buf_ptr = LCH_DeltaUnmarshal(&parent, instance, buf_ptr);
    if (buf_ptr == NULL) {
      return false;
    }
    assert(buf_ptr - buffer >= 0);
    assert(parent != NULL);

    const char *const table_id =
        LCH_TableDefinitionGetIdentifier(LCH_DeltaGetTable(parent));
    assert(table_id != NULL);

    if (!LCH_DictHasKey(deltas, table_id)) {
      LCH_LOG_ERROR(
          "Unmarshaled table with table ID '%s' not defined in leech instance.",
          table_id);
      LCH_DeltaDestroy(parent);
      return false;
    }

    LCH_Delta *const child = (LCH_Delta *)LCH_DictGet(deltas, table_id);
    if (child == NULL) {
      LCH_DeltaDestroy(parent);
      return false;
    }

    if (!LCH_DeltaCompress(child, parent)) {
      LCH_LOG_ERROR("Failed to compress deltas for table '%s'.", table_id);
      LCH_DeltaDestroy(parent);
    }

    *num_merged += LCH_DeltaGetNumMergedOperations(child);
    *num_canceled += LCH_DeltaGetNumCanceledOperations(child);

    const size_t num_insertions = LCH_DeltaGetNumInsertions(parent);
    const size_t num_deletions = LCH_DeltaGetNumDeletions(parent);
    const size_t num_modifications = LCH_DeltaGetNumUpdates(parent);
    LCH_LOG_VERBOSE(
        "Compressed %zu insertions, %zu deletions and %zu modifications for "
        "table '%s'.",
        num_insertions, num_deletions, num_modifications, table_id);

    LCH_DeltaDestroy(parent);
  }
  return true;
}

static LCH_Dict *EnumerateBlocks(const LCH_Instance *const instance,
                                 const char *const block_id,
                                 size_t *const num_merged,
                                 size_t *const num_canceled,
                                 size_t *const num_blocks) {
  assert(instance != NULL);
  assert(block_id != NULL);

  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  char *cursor = LCH_HeadGet("HEAD", work_dir);
  if (cursor == NULL) {
    LCH_LOG_ERROR("Failed to load head.");
    return NULL;
  }

  LCH_Dict *deltas = CreateEmptyDeltas(instance);
  if (deltas == NULL) {
    LCH_LOG_ERROR("Failed to initialize delta compression");
    free(cursor);
    return NULL;
  }

  *num_merged = 0;
  *num_canceled = 0;
  *num_blocks = 0;
  while (strcmp(cursor, block_id) != 0) {
    LCH_Block *const block = LCH_BlockLoad(work_dir, cursor);
    if (block == NULL) {
      LCH_LOG_ERROR("Failed to load block '%s'.", cursor);
      LCH_DictDestroy(deltas);
      free(cursor);
      return NULL;
    }
    LCH_LOG_VERBOSE("Loaded block '%s'.", cursor);

    const char *block_data = (char *)LCH_BlockGetData(block);
    assert(block_data != NULL);
    const size_t block_data_len = LCH_BlockGetDataLength(block);

    if (!CompressDeltas(deltas, instance, block_data, block_data_len,
                        num_merged, num_canceled)) {
      LCH_LOG_ERROR("Failed to compress deltas in block '%s'.", cursor);
      free(block);
      LCH_DictDestroy(deltas);
      free(cursor);
      return NULL;
    }

    free(cursor);
    cursor = LCH_BlockGetParentID(block);
    free(block);
    *num_blocks += 1;
  }

  free(cursor);

  return deltas;
}

char *LCH_Diff(const LCH_Instance *const instance, const char *const block_id,
               size_t *const buf_len) {
  assert(instance != NULL);
  assert(block_id != NULL);

  size_t num_merged, num_canceled, num_blocks;
  LCH_Dict *deltas = EnumerateBlocks(instance, block_id, &num_merged,
                                     &num_canceled, &num_blocks);
  if (deltas == NULL) {
    LCH_LOG_ERROR("Failed to enumerate blocks.");
    return NULL;
  }
  LCH_LOG_INFO(
      "Enumerated %zu block(s); %zu operations were merged, and %zu operations "
      "canceled each other out.",
      num_blocks, num_merged, num_canceled);

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    LCH_DictDestroy(deltas);
    return NULL;
  }

  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  char *const head = LCH_HeadGet("HEAD", work_dir);
  if (!LCH_BufferPrintFormat(buffer, "%s", head)) {
    free(head);
    LCH_BufferDestroy(buffer);
    LCH_DictDestroy(deltas);
    return NULL;
  }
  free(head);

  LCH_List *keys = LCH_DictGetKeys(deltas);
  if (keys == NULL) {
    LCH_BufferDestroy(buffer);
    LCH_DictDestroy(deltas);
    return NULL;
  }

  for (size_t i = 0; i < LCH_ListLength(keys); i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    LCH_Delta *delta = (LCH_Delta *)LCH_DictGet(deltas, key);
    assert(delta != NULL);

    if (!LCH_DeltaMarshal(buffer, delta)) {
      LCH_LOG_ERROR("Failed to marshal delta.");
      LCH_ListDestroy(keys);
      LCH_BufferDestroy(buffer);
      LCH_DictDestroy(deltas);
      return NULL;
    }
  }

  LCH_ListDestroy(keys);
  LCH_DictDestroy(deltas);

  char *result = (char *)malloc(LCH_BufferLength(buffer));
  if (result == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  memcpy(result, LCH_BufferGet(buffer, 0), LCH_BufferLength(buffer));
  *buf_len = LCH_BufferLength(buffer);
  LCH_BufferDestroy(buffer);

  return result;
}

static bool PatchTable(const LCH_Instance *const self,
                       const LCH_Delta *const delta,
                       const char *const uid_field,
                       const char *const uid_value) {
  assert(self != NULL);
  assert(delta != NULL);

  const LCH_TableDefinition *const table_def = LCH_DeltaGetTable(delta);
  if (table_def == NULL) {
    LCH_LOG_ERROR(
        "Table from patch with table id '%s' was not found in instance.",
        LCH_TableDefinitionGetIdentifier(table_def));
    return false;
  }

  return LCH_TableDefinitionPatch(table_def, delta, uid_field, uid_value);
}

bool LCH_Patch(const LCH_Instance *const instance, const char *const uid_field,
               const char *const uid_value, const char *const patch,
               const size_t size) {
  assert(instance != NULL);
  assert(patch != NULL);

  const char *buffer = patch;
  char head[SHA_DIGEST_LENGTH * 2 + 1];
  memcpy(head, buffer, SHA_DIGEST_LENGTH * 2);
  head[SHA_DIGEST_LENGTH * 2] = '\0';
  buffer += SHA_DIGEST_LENGTH * 2;

  size_t num_tables = 0, tot_insert = 0, tot_delete = 0, tot_update = 0;
  while ((size_t)(buffer - patch) < size) {
    LCH_Delta *delta;
    buffer = LCH_DeltaUnmarshal(&delta, instance, buffer);
    if (buffer == NULL) {
      LCH_LOG_ERROR("Failed to unmarshal patch.");
      return false;
    }

    if (!PatchTable(instance, delta, uid_field, uid_value)) {
      LCH_LOG_ERROR("Failed to patch table '%s'.",
                    LCH_TableDefinitionGetIdentifier(LCH_DeltaGetTable(delta)));
      LCH_DeltaDestroy(delta);
      return false;
    }

    const size_t num_insert = LCH_DeltaGetNumInsertions(delta);
    const size_t num_delete = LCH_DeltaGetNumDeletions(delta);
    const size_t num_update = LCH_DeltaGetNumUpdates(delta);
    tot_insert += num_insert;
    tot_delete += num_delete;
    tot_update += num_update;
    num_tables += 1;

    LCH_LOG_VERBOSE(
        "Patched table '%s' with %zu insertions, %zu deletions, and %zu "
        "updates.",
        LCH_TableDefinitionGetIdentifier(LCH_DeltaGetTable(delta)), num_insert,
        num_delete, num_update);
    LCH_DeltaDestroy(delta);
  }

  LCH_LOG_INFO(
      "Patched %zu tables with a total of %zu insertions, %zu deletions, and "
      "%zu updates",
      num_tables, tot_insert, tot_delete, tot_update);

  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  if (!LCH_HeadSet(uid_value, work_dir, head)) {
    return NULL;
  }

  return true;
}
