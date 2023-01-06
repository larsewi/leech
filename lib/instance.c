#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#include "dict.h"
#include "leech.h"
#include "table.h"
#include "utils.h"
#include "definitions.h"

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

  if (LCH_IsDirectory(instance->work_dir))
  {
    LCH_LOG_DEBUG("Directory '%s' already exists", instance->work_dir);
  }
  else {
    int ret = mkdir(instance->work_dir, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", instance->work_dir, strerror(errno));
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  char path[PATH_MAX];
  int ret = snprintf(path, sizeof(path), "%s%c%s", instance->work_dir, PATH_SEP, "snapshot");
  if (ret < 0 || (size_t) ret >= sizeof(path)) {
    LCH_LOG_ERROR("Failed to join paths '%s' and '%s': Trunctaion error", instance->work_dir, "snapshot");
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  if (LCH_IsDirectory(path)) {
    LCH_LOG_DEBUG("Directory '%s' already exists", path);
  }
  else {
    ret = mkdir(path, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", path, strerror(errno));
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  ret = snprintf(path, sizeof(path), "%s%c%s", instance->work_dir, PATH_SEP, "blocks");
  if (ret < 0 || (size_t) ret >= sizeof(path)) {
    LCH_LOG_ERROR("Failed to join paths '%s' and '%s': Trunctaion error", instance->work_dir, "blocks");
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  if (LCH_IsDirectory(path)) {
    LCH_LOG_DEBUG("Directory '%s' already exists", path);
  }
  else {
    ret = mkdir(path, S_IRWXU);
    if (ret != 0) {
      LCH_LOG_ERROR("Failed to create directory '%s': %s", path, strerror(errno));
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

  LCH_List *tables = self->tables;
  size_t num_tables = LCH_ListLength(tables);

  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(tables, i);
    const char *const table_id = LCH_TableGetIdentifier(table);

    LCH_LOG_DEBUG("Loading new data from table '%s'", table_id);
    LCH_Dict *new_data = LCH_TableLoadNewData(table);
    if (new_data == NULL) {
      LCH_LOG_ERROR("Failed to load new data from table '%s'", table_id);
      return false;
    }

    LCH_LOG_DEBUG("Loading old data from table '%s'", table_id);
    LCH_Dict *old_data = LCH_TableLoadOldData(table, self->work_dir);
    if (old_data == NULL) {
      LCH_LOG_ERROR("Failed to load old data from table '%s'", table_id);
      LCH_DictDestroy(new_data);
      return false;
    }

    LCH_DictDestroy(new_data);
    LCH_DictDestroy(old_data);
  }

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
