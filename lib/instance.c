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

static bool CalculateDiff(LCH_Buffer *const diff,
                          const LCH_Dict *const new_data,
                          const LCH_Dict *const old_data,
                          size_t *const n_additions, size_t *const n_deletions,
                          size_t *const n_modifications) {
  assert(diff != NULL);
  assert(new_data != NULL);
  assert(old_data != NULL);
  assert(n_additions != NULL);
  assert(n_deletions != NULL);
  assert(n_modifications != NULL);

  LCH_Dict *additions =
      LCH_DictSetMinus(new_data, old_data, (void *(*)(const void *))strdup);
  if (additions == NULL) {
    LCH_LOG_ERROR("Failed to calculate insertion entries.");
    return false;
  }
  *n_additions = LCH_DictLength(additions);

  LCH_DictIter *iter = LCH_DictIterCreate(additions);
  if (iter == NULL) {
    LCH_LOG_ERROR("Failed to create iterator for insertion entries.");
    LCH_DictDestroy(additions);
    return false;
  }

  while (LCH_DictIterNext(iter)) {
    const char *key = LCH_DictIterGetKey(iter);
    const char *value = (char *)LCH_DictIterGetValue(iter);
    if (!LCH_BufferAppend(diff, "+,%s,%s\r\n", key, value)) {
      LCH_LOG_ERROR("Failed to append insertion entries to diff buffer.");
      free(iter);
      LCH_DictDestroy(additions);
      return false;
    }
  }

  free(iter);
  LCH_DictDestroy(additions);

  /************************************************************************/

  LCH_Dict *deletions =
      LCH_DictSetMinus(old_data, new_data, (void *(*)(const void *))strdup);
  if (deletions == NULL) {
    LCH_LOG_ERROR("Failed to calculate deletion entries.");
    return false;
  }
  *n_deletions = LCH_DictLength(deletions);

  iter = LCH_DictIterCreate(deletions);
  if (iter == NULL) {
    LCH_LOG_ERROR("Failed to create iterator for deletion entries.");
    LCH_DictDestroy(deletions);
    return false;
  }

  while (LCH_DictIterNext(iter)) {
    const char *key = LCH_DictIterGetKey(iter);
    const char *value = (char *)LCH_DictIterGetValue(iter);
    if (!LCH_BufferAppend(diff, "-,%s,%s\r\n", key, value)) {
      LCH_LOG_ERROR("Failed to append deletion entries to diff buffer.");
      free(iter);
      LCH_DictDestroy(deletions);
      return false;
    }
  }

  free(iter);
  LCH_DictDestroy(deletions);

  /************************************************************************/

  LCH_Dict *modifications = LCH_DictSetChangedIntersection(
      new_data, old_data, (void *(*)(const void *))strdup,
      (int (*)(const void *, const void *))strcmp);
  if (modifications == NULL) {
    LCH_LOG_ERROR("Failed to calculate modifications entries.");
    return false;
  }
  *n_modifications = LCH_DictLength(modifications);

  iter = LCH_DictIterCreate(modifications);
  if (iter == NULL) {
    LCH_LOG_ERROR("Failed to create iterator for modifications entries.");
    LCH_DictDestroy(modifications);
    return false;
  }

  while (LCH_DictIterNext(iter)) {
    const char *key = LCH_DictIterGetKey(iter);
    const char *value = (char *)LCH_DictIterGetValue(iter);
    if (!LCH_BufferAppend(diff, "%%,%s,%s\r\n", key, value)) {
      LCH_LOG_ERROR("Failed to append modification entries to diff buffer.");
      free(iter);
      LCH_DictDestroy(modifications);
      return false;
    }
  }

  free(iter);
  LCH_DictDestroy(modifications);

  return true;
}

bool LCH_InstanceCommit(const LCH_Instance *const self) {
  assert(self != NULL);
  assert(self->tables != NULL);

  LCH_List *tables = self->tables;
  size_t num_tables = LCH_ListLength(tables);

  LCH_Buffer *diff = LCH_BufferCreate();
  if (diff == NULL) {
    return NULL;
  }

  size_t tot_insertions = 0, tot_deletions = 0, tot_modifications = 0;

  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(tables, i);

    if (i > 0) {
      if (!LCH_BufferAppend(diff, "\r\n")) {
        LCH_LOG_ERROR("Failed to add table separator.");
        LCH_BufferDestroy(diff);
        return false;
      }
    }

    /************************************************************************/

    const char *const table_id = LCH_TableGetIdentifier(table);
    char *composed_table_id = LCH_CSVComposeField(table_id);
    if (composed_table_id == NULL) {
      LCH_LOG_ERROR("Failed to compose table id for diffs for table '%s'.",
                    table_id);
      free(diff);
      return false;
    }

    if (!LCH_BufferAppend(diff, "%s\r\n", composed_table_id)) {
      LCH_LOG_ERROR(
          "Failed to append composed table id to diffs for table '%s'.",
          table_id);
      free(composed_table_id);
      free(diff);
      return false;
    }

    free(composed_table_id);

    /************************************************************************/

    LCH_Dict *new_state = LCH_TableLoadNewData(table);
    if (new_state == NULL) {
      LCH_LOG_ERROR("Failed to load new state from table '%s'.", table_id);
      LCH_BufferDestroy(diff);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded new state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(new_state));

    LCH_Dict *old_state = LCH_TableLoadOldData(table, self->work_dir);
    if (old_state == NULL) {
      LCH_LOG_ERROR("Failed to load old state from table '%s'.", table_id);
      LCH_BufferDestroy(diff);
      LCH_DictDestroy(new_state);
      return false;
    }
    LCH_LOG_VERBOSE("Loaded old state for table '%s' containing %zu rows.",
                    table_id, LCH_DictLength(new_state));

    /************************************************************************/
    size_t n_insertions, n_deletions, n_modifications;
    if (!CalculateDiff(diff, new_state, old_state, &n_insertions, &n_deletions,
                       &n_modifications)) {
      LCH_LOG_ERROR("Failed to compute delta for table '%s'.", table_id);
      LCH_BufferDestroy(diff);
      LCH_DictDestroy(new_state);
      LCH_DictDestroy(old_state);
      return false;
    }
    LCH_LOG_VERBOSE(
        "Computed delta for table '%s' including %zu insertions, %zu "
        "deletions, and %zu modifications.",
        table_id, n_insertions, n_deletions, n_modifications);

    tot_insertions += n_insertions;
    tot_deletions += n_deletions;
    tot_modifications += n_modifications;

    LCH_DictDestroy(old_state);

    /************************************************************************/

    LCH_LOG_VERBOSE("Creating new snapshot for table '%s'.", table_id);
    char path[PATH_MAX];
    if (!LCH_PathJoin(path, sizeof(path), 3, self->work_dir, "snapshot",
                      table_id)) {
      LCH_LOG_ERROR("Failed to create snapshot for table '%s'.", table_id);
      LCH_BufferDestroy(diff);
      LCH_DictDestroy(new_state);
      return false;
    }

    FILE *const file = fopen(path, "w");
    if (file == NULL) {
      LCH_LOG_ERROR(
          "Filed to create snapshot for table '%s': Failed to open file '%s': "
          "%s",
          table_id, strerror(errno));
      LCH_BufferDestroy(diff);
      LCH_DictDestroy(new_state);
      return false;
    }

    LCH_DictIter *iter = LCH_DictIterCreate(new_state);
    if (iter == NULL) {
      LCH_LOG_ERROR("Failed to create snapshot for table '%s'.", table_id);
      fclose(file);
      LCH_BufferDestroy(diff);
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

    free(iter);
    fclose(file);
    LCH_DictDestroy(new_state);

    /************************************************************************/
  }

  char *delta = LCH_BufferGet(diff);
  size_t delta_len = LCH_BufferLength(diff);
  LCH_BufferDestroy(diff);

  /**************************************************************************/

  char *const head = LCH_HeadGet(self->work_dir);
  if (head == NULL) {
    LCH_LOG_ERROR("Failed to get head.");
    free(delta);
    return false;
  }
  LCH_LOG_VERBOSE("Head positioned at '%s'.", head);

  LCH_Block *block = LCH_BlockCreate(head, delta, delta_len);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to create block.");
    free(head);
    free(delta);
    return false;
  }
  free(head);

  char *const block_id = LCH_BlockStore(self->work_dir, block);
  if (block_id == NULL) {
    LCH_LOG_ERROR("Failed to store block.");
    free(block);
    free(delta);
    return false;
  }
  free(block);
  free(delta);
  LCH_LOG_VERBOSE("Created block '%s'.", block_id);

  if (!LCH_HeadSet(self->work_dir, block_id)) {
    LCH_LOG_ERROR("Failed to move head to '%s'.", block_id);
    free(block_id);
    return false;
  }
  LCH_LOG_VERBOSE("Moved head to '%s'.", block_id);

  LCH_LOG_INFO(
      "Created block '%s' with a total delta of %zu insertions, "
      "%zu deletions, and %zu modifications, over %zu table(s).",
      block_id, tot_insertions, tot_deletions, tot_modifications,
      LCH_ListLength(tables));
  free(block_id);

  return true;
}

LCH_List *LCH_InstanceGetTables(const LCH_Instance *const instance) {
  return instance->tables;
}

// static LCH_Dict *CreateEmptyDiffs(LCH_Instance *instance) {
//   assert(instance != NULL);
//   assert(instance->tables != NULL);

//   LCH_Dict *diffs = LCH_DictCreate();
//   if (diffs == NULL) {
//     return NULL;
//   }

//   size_t n_tables = LCH_ListLength(instance->tables);
//   for (size_t i = 0; i < n_tables; i++) {
//     LCH_Table *table = LCH_ListGet(instance->tables, i);
//     assert(table != NULL);
//     const char *const table_id = LCH_TableGetIdentifier(table);

//     LCH_Dict *diff = LCH_DictCreate();
//     if (diff == NULL) {
//       LCH_DictDestroy(diffs);
//       return NULL;
//     }

//     if (!LCH_DictSet(diffs, table_id, diff, (void (*)(void *))LCH_DictDestroy)) {
//       LCH_DictDestroy(diff);
//       LCH_DictDestroy(diffs);
//       return NULL;
//     }
//   }

//   return diffs;
// }

// static LCH_Dict *ExtractDiffsFromDelta(LCH_Instance *instance, const char *const delta) {
//   assert(delta != NULL);

//   LCH_Dict *const diffs = CreateEmptyDiffs(instance);
//   if (diffs == NULL) {
//     return NULL;
//   }

//   return diffs;
// }

// static bool EnumerateBlocks(const LCH_Instance *const instance, const char *const block_id) {
//   assert(instance != NULL);
//   assert(instance->work_dir != NULL);
//   assert(block_id != NULL);

//   char *cursor = LCH_HeadGet(instance->work_dir);
//   if (cursor == NULL) {
//     LCH_LOG_ERROR("Failed to load head.");
//     return NULL;
//   }

//   LCH_Dict *compressed = CreateEmptyDiffs(instance);
//   if (compressed == NULL) {
//     return NULL;
//   }

//   while (strcmp(cursor, LCH_GENISIS_BLOCK_PARENT) != 0) {
//     LCH_Block *const block = LCH_BlockLoad(instance->work_dir, cursor);
//     if (block == NULL) {
//       free(cursor);
//       return NULL;
//     }

//     const char *const delta = LCH_BlockGetData(block);
//     LCH_Dict *const cursor_diffs = ExtractDiffsFromDelta(instance, delta);

//     free(cursor);
//     cursor = LCH_BlockGetParentID(block);
//     free(block);
//   }

//   free(cursor);
// }

char *LCH_InstanceDiff(const LCH_Instance *const self, const char *const block_id) {
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
