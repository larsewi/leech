#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <memory.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "block.h"
#include "buffer.h"
#include "csv.h"
#include "definitions.h"
#include "delta.h"
#include "dict.h"
#include "head.h"
#include "leech.h"
#include "table.h"
#include "utils.h"

struct LCH_Instance {
  const char *work_dir;
  LCH_List *tables;
};

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->work_dir != NULL);

  LCH_Instance *instance = (LCH_Instance *)malloc(sizeof(LCH_Instance));
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for instance: %s",
                  strerror(errno));
    return NULL;
  }

  instance->work_dir = createInfo->work_dir;
  instance->tables = LCH_ListCreate();
  if (instance->tables == NULL) {
    free(instance);
    return NULL;
  }

  if (!LCH_IsDirectory(instance->work_dir)) {
    LCH_LOG_VERBOSE("Creating directory '%s'.", createInfo->work_dir);
    const int ret = mkdir(instance->work_dir, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", instance->work_dir,
                    strerror(errno));
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 2, instance->work_dir, "snapshot")) {
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  if (!LCH_IsDirectory(path)) {
    LCH_LOG_VERBOSE("Creating directory '%s'.", path);
    const int ret = mkdir(path, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", path,
                    strerror(errno));
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  if (!LCH_PathJoin(path, sizeof(path), 2, instance->work_dir, "blocks")) {
    LCH_LOG_ERROR("Failed to join paths '%s' and '%s': Trunctaion error",
                  instance->work_dir, "blocks");
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  if (!LCH_IsDirectory(path)) {
    LCH_LOG_VERBOSE("Creating directory '%s'.", path);
    const int ret = mkdir(path, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", path,
                    strerror(errno));
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  return instance;
}

bool LCH_InstanceAddTable(LCH_Instance *const instance,
                          LCH_Table *const table) {
  assert(instance != NULL);
  assert(instance->tables != NULL);
  assert(table != NULL);

  return LCH_ListAppend(instance->tables, table,
                        (void (*)(void *))LCH_TableDestroy);
}

const LCH_Table *LCH_InstanceGetTable(const LCH_Instance *const self,
                                      const char *const table_id) {
  assert(self != NULL);
  assert(self->tables != NULL);
  assert(table_id != NULL);

  const size_t num_tables = LCH_ListLength(self->tables);
  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(self->tables, i);
    assert(table != NULL);

    if (strcmp(LCH_TableGetIdentifier(table), table_id) == 0) {
      return table;
    }
  }
  return NULL;
}

bool LCH_InstanceCommit(const LCH_Instance *const self) {
  assert(self != NULL);
  assert(self->tables != NULL);

  LCH_List *tables = self->tables;
  size_t n_tables = LCH_ListLength(tables);

  LCH_Buffer *delta_buffer = LCH_BufferCreate();
  if (delta_buffer == NULL) {
    return NULL;
  }

  size_t tot_ins = 0, tot_del = 0, tot_mod = 0;

  for (size_t i = 0; i < n_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(tables, i);
    const char *const table_id = LCH_TableGetIdentifier(table);

    /************************************************************************/

    LCH_Dict *new_state = LCH_TableLoadNewState(table);
    if (new_state == NULL) {
      LCH_LOG_ERROR("Failed to load new state for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded new state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(new_state));

    LCH_Dict *old_state = LCH_TableLoadOldState(table, self->work_dir);
    if (old_state == NULL) {
      LCH_LOG_ERROR("Failed to load old state for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(new_state);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded old state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(old_state));

    /************************************************************************/

    LCH_Delta *const delta = LCH_DeltaCreate(table, new_state, old_state);
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

    if (!LCH_TableStoreNewState(table, self->work_dir, new_state)) {
      LCH_LOG_ERROR("Failed to store new state for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(new_state);
      return false;
    }
    LCH_LOG_VERBOSE("Stored new state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(new_state));
  }

  char *const head = LCH_HeadGet("HEAD", self->work_dir);
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

  char *const block_id = LCH_BlockStore(self->work_dir, block);
  if (block_id == NULL) {
    LCH_LOG_ERROR("Failed to store block.");
    free(block);
    free(head);
    return false;
  }
  free(block);
  LCH_LOG_INFO(
      "Created block '%.5s' with a deltas containing a total of %zu "
      "insertions, "
      "%zu deletions, and %zu modifications, over %zu table(s).",
      block_id, tot_ins, tot_del, tot_mod, LCH_ListLength(tables));

  if (!LCH_HeadSet("HEAD", self->work_dir, block_id)) {
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
  assert(instance->tables != NULL);

  LCH_Dict *const deltas = LCH_DictCreate();
  if (deltas == NULL) {
    return NULL;
  }

  const size_t num_tables = LCH_ListLength(instance->tables);
  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(instance->tables, i);
    assert(table != NULL);

    LCH_Delta *const delta = LCH_DeltaCreate(table, NULL, NULL);
    if (delta == NULL) {
      LCH_DictDestroy(deltas);
      return NULL;
    }

    if (!LCH_DictSet(deltas, LCH_TableGetIdentifier(table), delta,
                     (void (*)(void *))LCH_DeltaDestroy)) {
      LCH_DeltaDestroy(delta);
      LCH_DictDestroy(deltas);
      return NULL;
    }
  }

  return deltas;
}

static bool CompressDeltas(LCH_Dict *const deltas,
                           const LCH_Instance *const instance,
                           const char *const buffer, const size_t buf_len) {
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
        LCH_TableGetIdentifier(LCH_DeltaGetTable(parent));
    assert(table_id != NULL);

    if (!LCH_DictHasKey(deltas, table_id)) {
      LCH_LOG_ERROR(
          "Unmarshaled table with table ID '%s' not defined in leech instance.",
          table_id);
      LCH_DeltaDestroy(parent);
      return false;
    }

    LCH_Delta *const child = LCH_DictGet(deltas, table_id);
    if (child == NULL) {
      LCH_DeltaDestroy(parent);
      return false;
    }

    if (!LCH_DeltaCompress(child, parent)) {
      LCH_LOG_ERROR("Failed to compress deltas for table '%s'.", table_id);
      LCH_DeltaDestroy(parent);
    }

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
                                 const char *const block_id) {
  assert(instance != NULL);
  assert(instance->work_dir != NULL);
  assert(block_id != NULL);

  char *cursor = LCH_HeadGet("HEAD", instance->work_dir);
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

  size_t enumerated_blocks = 0;
  while (strcmp(cursor, block_id) != 0) {
    LCH_Block *const block = LCH_BlockLoad(instance->work_dir, cursor);
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

    if (!CompressDeltas(deltas, instance, block_data, block_data_len)) {
      LCH_LOG_ERROR("Failed to compress deltas in block '%s'.", cursor);
      free(block);
      LCH_DictDestroy(deltas);
      free(cursor);
      return NULL;
    }

    free(cursor);
    cursor = LCH_BlockGetParentID(block);
    free(block);
    enumerated_blocks += 1;
  }

  LCH_LOG_VERBOSE("Enumerated %zu blocks.", enumerated_blocks);
  free(cursor);

  return deltas;
}

char *LCH_InstanceDelta(const LCH_Instance *const self,
                        const char *const block_id, size_t *const buf_len) {
  assert(self != NULL);
  assert(block_id != NULL);

  LCH_Dict *deltas = EnumerateBlocks(self, block_id);
  if (deltas == NULL) {
    LCH_LOG_ERROR("Failed to enumerate blocks.");
    return NULL;
  }

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    LCH_DictDestroy(deltas);
    return NULL;
  }

  char *const head = LCH_HeadGet("HEAD", self->work_dir);
  if (!LCH_BufferPrintFormat(buffer, head)) {
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
    const char *const key = LCH_ListGet(keys, i);
    assert(key != NULL);

    LCH_Delta *delta = LCH_DictGet(deltas, key);
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

  char *result = malloc(LCH_BufferLength(buffer));
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

  const LCH_Table *const table = LCH_DeltaGetTable(delta);
  if (table == NULL) {
    LCH_LOG_ERROR(
        "Table from patch with table id '%s' was not found in instance.",
        LCH_TableGetIdentifier(table));
    return false;
  }

  return LCH_TablePatch(table, delta, uid_field, uid_value);
}

bool LCH_InstancePatch(const LCH_Instance *const self,
                       const char *const uid_field, const char *const uid_value,
                       const char *const patch, const size_t size) {
  assert(self != NULL);
  assert(patch != NULL);

  const char *buffer = patch;
  char head[SHA_DIGEST_LENGTH * 2 + 1];
  memcpy(head, buffer, SHA_DIGEST_LENGTH * 2);
  head[SHA_DIGEST_LENGTH * 2] = '\0';
  buffer += SHA_DIGEST_LENGTH * 2;

  while ((size_t)(buffer - patch) < size) {
    LCH_Delta *delta;
    buffer = LCH_DeltaUnmarshal(&delta, self, buffer);
    if (buffer == NULL) {
      LCH_LOG_ERROR("Failed to unmarshal patch.");
      return false;
    }

    if (!PatchTable(self, delta, uid_field, uid_value)) {
      LCH_LOG_ERROR("Failed to patch table '%s'.",
                    LCH_TableGetIdentifier(LCH_DeltaGetTable(delta)));
      LCH_DeltaDestroy(delta);
      return false;
    }
    LCH_LOG_DEBUG("Patched table '%s'.",
                  LCH_TableGetIdentifier(LCH_DeltaGetTable(delta)));
    LCH_DeltaDestroy(delta);
  }

  if (!LCH_HeadSet(uid_value, self->work_dir, head)) {
    return NULL;
  }

  return true;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  LCH_ListDestroy(instance->tables);
  free(instance);
}
