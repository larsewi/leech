#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <memory.h>
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
  const char *identifier;
  const char *work_dir;
  LCH_List *tables;
};

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->identifier != NULL);
  assert(createInfo->work_dir != NULL);

  LCH_Instance *instance = (LCH_Instance *)malloc(sizeof(LCH_Instance));
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for instance: %s",
                  strerror(errno));
    return NULL;
  }

  instance->identifier = createInfo->identifier;
  instance->work_dir = createInfo->work_dir;
  instance->tables = LCH_ListCreate();
  if (instance->tables == NULL) {
    free(instance);
    return NULL;
  }

  if (LCH_IsDirectory(instance->work_dir)) {
    LCH_LOG_DEBUG("Directory '%s' already exists", instance->work_dir);
  } else {
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

  if (LCH_IsDirectory(path)) {
    LCH_LOG_DEBUG("Directory '%s' already exists", path);
  } else {
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

  if (LCH_IsDirectory(path)) {
    LCH_LOG_DEBUG("Directory '%s' already exists", path);
  } else {
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

    LCH_Delta *const delta = LCH_DeltaCreate(table_id, new_state, old_state);
    if (delta == NULL) {
      LCH_LOG_ERROR("Failed to compute delta for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(old_state);
      LCH_DictDestroy(new_state);
      return false;
    }

    const size_t num_ins = LCH_DeltaGetNumInsertions(delta);
    const size_t num_del = LCH_DeltaGetNumDeletions(delta);
    const size_t num_mod = LCH_DeltaGetNumModifications(delta);
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

    char path[PATH_MAX];
    if (!LCH_PathJoin(path, sizeof(path), 3, self->work_dir, "snapshot",
                      table_id)) {
      LCH_LOG_ERROR("Failed to create snapshot for table '%s'.", table_id);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(new_state);
      return false;
    }

    FILE *const file = fopen(path, "w");
    if (file == NULL) {
      LCH_LOG_ERROR(
          "Filed to create snapshot for table '%s': Failed to open file '%s': "
          "%s",
          table_id, strerror(errno));
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(new_state);
      return false;
    }

    LCH_DictIter *iter = LCH_DictIterCreate(new_state);
    if (iter == NULL) {
      LCH_LOG_ERROR("Failed to create snapshot for table '%s'.", table_id);
      fclose(file);
      LCH_BufferDestroy(delta_buffer);
      LCH_DictDestroy(new_state);
      return false;
    }

    while (LCH_DictIterNext(iter)) {
      const char *const key = LCH_DictIterGetKey(iter);
      assert(key != NULL);
      assert(fprintf(file, "%s\r\n", key) > 0);

      const char *const value = (char *)LCH_DictIterGetValue(iter);
      assert(fprintf(file, "%s\r\n", value) > 0);
      assert(value != NULL);
    }

    LCH_LOG_VERBOSE("Created new snapshot for table '%s'.", table_id);

    free(iter);
    fclose(file);
    LCH_DictDestroy(new_state);
  }

  char *const head = LCH_HeadGet(self->work_dir);
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
      "Created block '%s' with a deltas containing a total of %zu insertions, "
      "%zu deletions, and %zu modifications, over %zu table(s).",
      block_id, tot_ins, tot_del, tot_mod, LCH_ListLength(tables));

  if (!LCH_HeadSet(self->work_dir, block_id)) {
    LCH_LOG_ERROR("Failed to move head to '%s'.", block_id);
    free(head);
    free(block_id);
    return false;
  }
  LCH_LOG_VERBOSE("Moved head from '%s' to '%s'.", head, block_id);

  free(head);
  free(block_id);
  return true;
}

LCH_Dict *CreateEmptyDeltas(const LCH_Instance *const instance) {
  assert(instance != NULL);
  assert(instance->tables != NULL);

  LCH_Dict *const deltas = LCH_DictCreate();
  if (deltas == NULL) {
    return NULL;
  }

  const size_t num_tables = LCH_DictLength(instance->tables);
  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(instance->tables, i);
    assert(table != NULL);

    const char *const table_id = LCH_TableGetIdentifier(table);
    assert(table_id != NULL);

    LCH_Delta *const delta = LCH_DeltaCreate(table_id, NULL, NULL);
    if (delta == NULL) {
      LCH_DictDestroy(deltas);
      return NULL;
    }

    if (!LCH_DictSet(deltas, table_id, delta, LCH_DeltaDestroy)) {
      LCH_DeltaDestroy(delta);
      LCH_DictDestroy(deltas);
      return NULL;
    }
  }

  return deltas;
}

static bool CompressDeltas(LCH_Dict *const deltas, const char *const buffer, const size_t buf_len) {
  const char *buf_ptr = buffer;

  while (buf_ptr - buffer < buf_len) {
    LCH_Delta *child = NULL;
    buf_ptr = LCH_DeltaUnmarshal(&child, buf_ptr);
    if (buf_ptr == NULL) {
      return false;
    }
    assert(child != NULL);

    const char *const table_id = LCH_DeltaGetTableID(child);
    assert(table_id != NULL);

    if (!LCH_DictHasKey(deltas, table_id)) {
      LCH_LOG_ERROR("Unmarshaled table with table ID '%s' not defined in leech instance.", table_id);
      LCH_DeltaDestroy(child);
      return false;
    }

    LCH_Delta *const parent = LCH_DictGet(deltas, table_id);
    if (parent == NULL) {
      LCH_DeltaDestroy(child);
      return false;
    }
  }
}

static bool EnumerateBlocks(const LCH_Instance *const instance,
                            const char *const block_id) {
  assert(instance != NULL);
  assert(instance->work_dir != NULL);
  assert(block_id != NULL);

  char *cursor = LCH_HeadGet(instance->work_dir);
  if (cursor == NULL) {
    LCH_LOG_ERROR("Failed to load head.");
    return false;
  }

  LCH_Dict *deltas = CreateEmptyDeltas(instance);
  if (deltas == NULL) {
    LCH_LOG_ERROR("Failed to initialize delta compression");
    free(cursor);
    return false;
  }

  while (strcmp(cursor, LCH_GENISIS_BLOCK_PARENT) != 0) {
    LCH_Block *const block = LCH_BlockLoad(instance->work_dir, cursor);
    if (block == NULL) {
      LCH_DictDestroy(deltas);
      free(cursor);
      return false;
    }

    const char *block_data = (char *)LCH_BlockGetData(block);
    assert(block_data != NULL);
    const size_t block_data_len = LCH_BlockGetDataLength(block);

    if (!CompressDeltas(deltas, block_data, block_data_len)) {
      free(block);
      LCH_DictDestroy(deltas);
      free(cursor);
      return false;
    }

    free(cursor);
    cursor = LCH_BlockGetParentID(block);
    free(block);
  }

  free(cursor);
}

char *LCH_InstanceDiff(const LCH_Instance *const self,
                       const char *const block_id) {
  assert(self != NULL);
  assert(block_id != NULL);

  return NULL;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  LCH_ListDestroy(instance->tables);
  free(instance);
}
