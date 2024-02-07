#include "instance.h"

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
#include "list.h"
#include "table.h"
#include "utils.h"

struct LCH_InstanceNew {
  char *work_dir;
  size_t major;
  size_t minor;
  size_t patch;
  LCH_List *tables;
};

void LCH_InstanceNewDestroy(void *const _instance) {
  LCH_InstanceNew *const instance = (LCH_InstanceNew *)_instance;
  free(instance->work_dir);
  LCH_ListDestroy(instance->tables);
  free(instance);
}

LCH_InstanceNew *LCH_InstanceLoad(const char *const work_dir) {
  assert(work_dir != NULL);

  char *const path =
      LCH_StringFormat("%s%c%s", work_dir, PATH_SEP, "leech.json");
  if (path == NULL) {
    return NULL;
  }

  char *const raw = LCH_FileRead(path, NULL);
  free(path);
  if (raw == NULL) {
    return NULL;
  }

  LCH_Json *const config = LCH_JsonParse(raw);
  free(raw);
  if (config == NULL) {
    return NULL;
  }

  LCH_InstanceNew *const instance =
      (LCH_InstanceNew *)malloc(sizeof(LCH_InstanceNew));
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for leech instance: %s",
                  strerror(errno));
    LCH_JsonDestroy(config);
    return NULL;
  }

  instance->work_dir = LCH_StringDuplicate(work_dir);
  if (instance->work_dir == NULL) {
    LCH_InstanceNewDestroy(instance);
    LCH_JsonDestroy(config);
    return NULL;
  }

  const char *const version = LCH_JsonObjectGetString(config, "version");
  assert(version != NULL);
  if (!LCH_ParseVersion(version, &instance->major, &instance->minor,
                        &instance->patch)) {
    LCH_InstanceNewDestroy(instance);
    LCH_JsonDestroy(config);
    return NULL;
  }

  const LCH_Json *const table_definitions =
      LCH_JsonObjectGetObject(config, "tables");
  assert(table_definitions != NULL);

  LCH_List *const table_ids = LCH_JsonObjectGetKeys(table_definitions);
  const size_t num_tables = LCH_ListLength(table_ids);

  instance->tables = LCH_ListCreateWithCapacity(num_tables);
  if (instance->tables == NULL) {
    LCH_ListDestroy(table_ids);
    LCH_InstanceNewDestroy(instance);
    return NULL;
  }

  for (size_t i = 0; i < num_tables; i++) {
    const char *const table_id = (char *)LCH_ListGet(table_ids, i);
    assert(table_id != NULL);

    const LCH_Json *const table_definition =
        LCH_JsonObjectGetObject(table_definitions, table_id);
    LCH_TableInfo *const table_info =
        LCH_TableInfoLoad(table_id, table_definition);
    if (table_info == NULL) {
      LCH_ListDestroy(table_ids);
      LCH_InstanceNewDestroy(instance);
      LCH_JsonDestroy(config);
      return NULL;
    }

    if (!LCH_ListAppend(instance->tables, table_info, LCH_TableInfoDestroy)) {
      LCH_TableInfoDestroy(table_info);
      LCH_ListDestroy(table_ids);
      LCH_InstanceNewDestroy(instance);
      LCH_JsonDestroy(config);
      return NULL;
    }
  }
  LCH_ListDestroy(table_ids);
  LCH_JsonDestroy(config);

  return instance;
}

struct LCH_Instance {
  const char *work_dir;
  LCH_List *table_defs;
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
  instance->table_defs = LCH_ListCreate();
  if (instance->table_defs == NULL) {
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

bool LCH_InstanceAddTableDefinition(LCH_Instance *const instance,
                                    LCH_TableDefinition *const table_def) {
  assert(instance != NULL);
  assert(instance->table_defs != NULL);
  assert(table_def != NULL);

  return LCH_ListAppend(instance->table_defs, table_def,
                        LCH_TableDefinitionDestroy);
}

const LCH_TableDefinition *LCH_InstanceGetTable(const LCH_Instance *const self,
                                                const char *const table_id) {
  assert(self != NULL);
  assert(self->table_defs != NULL);
  assert(table_id != NULL);

  const size_t num_tables = LCH_ListLength(self->table_defs);
  for (size_t i = 0; i < num_tables; i++) {
    const LCH_TableDefinition *const table_def =
        (LCH_TableDefinition *)LCH_ListGet(self->table_defs, i);
    assert(table_def != NULL);

    if (strcmp(LCH_TableDefinitionGetIdentifier(table_def), table_id) == 0) {
      return table_def;
    }
  }
  return NULL;
}

const LCH_List *LCH_InstanceGetTables(const LCH_Instance *const self) {
  assert(self != NULL);
  return self->table_defs;
}

const char *LCH_InstanceGetWorkDirectory(const LCH_Instance *const self) {
  assert(self != NULL);
  return self->work_dir;
}

void LCH_InstanceDestroy(void *const self) {
  LCH_Instance *const instance = (LCH_Instance *)self;
  if (instance == NULL) {
    return;
  }
  LCH_ListDestroy(instance->table_defs);
  free(instance);
}
