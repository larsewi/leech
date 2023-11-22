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
    const LCH_Table *const table = (LCH_Table *)LCH_ListGet(self->tables, i);
    assert(table != NULL);

    if (strcmp(LCH_TableGetIdentifier(table), table_id) == 0) {
      return table;
    }
  }
  return NULL;
}

const LCH_List *LCH_InstanceGetTables(const LCH_Instance *const self) {
  assert(self != NULL);
  return self->tables;
}

const char *LCH_InstanceGetWorkDirectory(const LCH_Instance *const self) {
  assert(self != NULL);
  return self->work_dir;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  LCH_ListDestroy(instance->tables);
  free(instance);
}
