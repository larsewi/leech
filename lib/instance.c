#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "buffer.h"
#include "csv.h"
#include "definitions.h"
#include "dict.h"
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
    int ret = mkdir(instance->work_dir, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", instance->work_dir,
                    strerror(errno));
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  char path[PATH_MAX];
  int ret = snprintf(path, sizeof(path), "%s%c%s", instance->work_dir, PATH_SEP,
                     "snapshot");
  if (ret < 0 || (size_t)ret >= sizeof(path)) {
    LCH_LOG_ERROR("Failed to join paths '%s' and '%s': Trunctaion error",
                  instance->work_dir, "snapshot");
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  if (LCH_IsDirectory(path)) {
    LCH_LOG_DEBUG("Directory '%s' already exists", path);
  } else {
    LCH_LOG_VERBOSE("Creating directory '%s'.", path);
    ret = mkdir(path, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", path,
                    strerror(errno));
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  ret = snprintf(path, sizeof(path), "%s%c%s", instance->work_dir, PATH_SEP,
                 "blocks");
  if (ret < 0 || (size_t)ret >= sizeof(path)) {
    LCH_LOG_ERROR("Failed to join paths '%s' and '%s': Trunctaion error",
                  instance->work_dir, "blocks");
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  if (LCH_IsDirectory(path)) {
    LCH_LOG_DEBUG("Directory '%s' already exists", path);
  } else {
    LCH_LOG_VERBOSE("Creating directory '%s'.", path);
    ret = mkdir(path, S_IRWXU);
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
                          size_t *const tot_additions,
                          size_t *const tot_deletions,
                          size_t *const tot_modifications) {
  LCH_Dict *additions =
      LCH_DictSetMinus(new_data, old_data, (void *(*)(const void *))strdup);
  if (additions == NULL) {
    LCH_LOG_ERROR("Failed to calculate additions.");
    return false;
  }
  const size_t n_additions = LCH_DictLength(additions);
  *tot_additions += n_additions;

  LCH_DictIter *iter = LCH_DictIterCreate(additions);
  if (iter == NULL) {
    LCH_LOG_ERROR("Failed to create iterator for addition entries.");
    LCH_DictDestroy(additions);
    return false;
  }

  while (LCH_DictIterNext(iter)) {
    const char *key = LCH_DictIterGetKey(iter);
    const char *value = (char *)LCH_DictIterGetValue(iter);
    if (!LCH_BufferAppend(diff, "+,%s,%s\r\n", key, value)) {
      LCH_LOG_ERROR("Failed to append addition entries to diff buffer.");
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
  const size_t n_deletions = LCH_DictLength(deletions);
  *tot_deletions += n_deletions;

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
  const size_t n_modifications = LCH_DictLength(modifications);
  *tot_modifications += n_modifications;

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

  /************************************************************************/

  LCH_LOG_DEBUG(
      "Calculated diff including %zu additions, %zu modifications and %zu "
      "deletions.",
      n_additions, n_deletions, n_modifications);

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

  size_t tot_additions = 0;
  size_t tot_deletions = 0;
  size_t tot_modifications = 0;

  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(tables, i);
    const char *const table_id = LCH_TableGetIdentifier(table);

    /************************************************************************/

    char *composed_table_id = LCH_CSVComposeField(table_id);
    if (composed_table_id == NULL) {
      LCH_LOG_ERROR("Failed to compose table id for diffs for table '%s'.", table_id);
      free(diff);
      return false;
    }

    if (!LCH_BufferAppend(diff, "%s\r\n", composed_table_id)) {
      LCH_LOG_ERROR("Failed to append composed table id to diffs for table '%s'.", table_id);
      free(composed_table_id);
      free(diff);
      return false;
    }

    free(composed_table_id);

    /************************************************************************/

    LCH_LOG_VERBOSE("Loading new state for table '%s'.", table_id);
    LCH_Dict *new_data = LCH_TableLoadNewData(table);
    if (new_data == NULL) {
      LCH_LOG_ERROR("Failed to load new data from table '%s'", table_id);
      LCH_BufferDestroy(diff);
      return false;
    }

    LCH_LOG_VERBOSE("Loading old state for table '%s'.", table_id);
    LCH_Dict *old_data = LCH_TableLoadOldData(table, self->work_dir);
    if (old_data == NULL) {
      LCH_LOG_ERROR("Failed to load old data from table '%s'", table_id);
      LCH_BufferDestroy(diff);
      LCH_DictDestroy(new_data);
      return false;
    }

    /************************************************************************/

    LCH_LOG_VERBOSE("Calculating diff for table '%s'", table_id);
    if (!CalculateDiff(diff, new_data, old_data, &tot_additions, &tot_deletions,
                       &tot_modifications)) {
      LCH_LOG_ERROR("Failed to calculate diff for table '%s'", table_id);
      LCH_BufferDestroy(diff);
      LCH_DictDestroy(new_data);
      LCH_DictDestroy(old_data);
      return false;
    }

    LCH_DictDestroy(old_data);

    /************************************************************************/

    LCH_DictDestroy(new_data);
  }

  LCH_LOG_INFO(
      "Calculated diffs including a total of %zu additions, %zu modifications "
      "and %zu deletions for %zu tables.",
      tot_additions, tot_deletions, tot_modifications, LCH_ListLength(tables));

  char *diff_str = LCH_BufferGet(diff);
  LCH_BufferDestroy(diff);
  LCH_LOG_DEBUG("Diff string: '%s'\n", diff_str);
  free(diff_str);

  return true;
}

LCH_List *LCH_InstanceGetTables(const LCH_Instance *const instance) {
  return instance->tables;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  LCH_ListDestroy(instance->tables);
  free(instance);
}
