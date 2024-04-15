#include "instance.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "files.h"
#include "list.h"
#include "logger.h"
#include "string_lib.h"
#include "table.h"
#include "utils.h"

struct LCH_Instance {
  const char *work_dir;
  size_t major;
  size_t minor;
  size_t patch;
  size_t max_chain_length;
  bool pretty_print;
  LCH_List *tables;
};

void LCH_InstanceDestroy(void *const _instance) {
  LCH_Instance *const instance = (LCH_Instance *)_instance;
  LCH_ListDestroy(instance->tables);
  free(instance);
}

LCH_Instance *LCH_InstanceLoad(const char *const work_dir) {
  assert(work_dir != NULL);

  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, PATH_MAX, 2, work_dir, "leech.json")) {
    return NULL;
  }

  LCH_Json *const config = LCH_JsonParseFile(path);
  if (config == NULL) {
    return NULL;
  }

  LCH_Instance *const instance = (LCH_Instance *)malloc(sizeof(LCH_Instance));
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for leech instance: %s",
                  strerror(errno));
    LCH_JsonDestroy(config);
    return NULL;
  }

  instance->work_dir = work_dir;

  {
    const LCH_Buffer *const key = LCH_BufferStaticFromString("version");
    const LCH_Buffer *const value = LCH_JsonObjectGetString(config, key);
    if (value == NULL) {
      LCH_InstanceDestroy(instance);
      LCH_JsonDestroy(config);
      return NULL;
    }

    const char *const version = LCH_BufferData(value);
    LCH_LOG_DEBUG("config[\"version\"] = \"%s\"", version);

    if (!LCH_StringParseVersion(version, &instance->major, &instance->minor,
                                &instance->patch)) {
      LCH_InstanceDestroy(instance);
      LCH_JsonDestroy(config);
      return NULL;
    }
  }

  {
    const LCH_Buffer *const key =
        LCH_BufferStaticFromString("max_chain_length");
    if (LCH_JsonObjectHasKey(config, key)) {
      double number;
      if (!LCH_JsonObjectGetNumber(config, key, &number)) {
        LCH_InstanceDestroy(instance);
        LCH_JsonDestroy(config);
        return NULL;
      }
      if (!LCH_DoubleToSize(number, &(instance->max_chain_length))) {
        LCH_InstanceDestroy(instance);
        LCH_JsonDestroy(config);
        return NULL;
      }
    } else {
      instance->max_chain_length = LCH_DEFAULT_MAX_CHAIN_LENGTH;
    }
    LCH_LOG_DEBUG("config[\"max_chain_length\"] = \"%zu\"",
                  instance->max_chain_length);
  }

  {
    instance->pretty_print = false;  // False by default
    const LCH_Buffer *const key = LCH_BufferStaticFromString("pretty_print");
    if (LCH_JsonObjectHasKey(config, key)) {
      const LCH_Json *const pretty_print = LCH_JsonObjectGet(config, key);
      if (pretty_print == NULL) {
        LCH_InstanceDestroy(instance);
        LCH_JsonDestroy(config);
        return NULL;
      }
      if (LCH_JsonIsTrue(pretty_print)) {
        instance->pretty_print = true;
      } else if (!LCH_JsonIsFalse(pretty_print)) {
        const char *const type = LCH_JsonGetTypeAsString(pretty_print);
        LCH_LOG_ERROR(
            "Illegal value for config[\"pretty_print\"]: "
            "Expected type true or false, found %s",
            type);
        LCH_InstanceDestroy(instance);
        LCH_JsonDestroy(config);
        return NULL;
      }
    }
    LCH_LOG_DEBUG("config[\"pretty_print\"] = %s",
                  (instance->pretty_print) ? "true" : "false");
  }

  const LCH_Buffer *const key = LCH_BufferStaticFromString("tables");
  const LCH_Json *const table_defs = LCH_JsonObjectGetObject(config, key);
  if (table_defs == NULL) {
    LCH_InstanceDestroy(instance);
    LCH_JsonDestroy(config);
    return NULL;
  }

  LCH_List *const table_ids = LCH_JsonObjectGetKeys(table_defs);
  if (table_ids == NULL) {
    LCH_InstanceDestroy(instance);
    LCH_JsonDestroy(config);
    return NULL;
  }

  instance->tables = LCH_ListCreate();
  if (instance->tables == NULL) {
    LCH_ListDestroy(table_ids);
    LCH_InstanceDestroy(instance);
    return NULL;
  }

  const size_t num_tables = LCH_ListLength(table_ids);
  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Buffer *const table_id = (LCH_Buffer *)LCH_ListGet(table_ids, i);
    assert(table_id != NULL);

    const LCH_Json *const table_definition =
        LCH_JsonObjectGetObject(table_defs, table_id);
    if (table_definition == NULL) {
      LCH_ListDestroy(table_ids);
      LCH_InstanceDestroy(instance);
      LCH_JsonDestroy(config);
      return NULL;
    }

    LCH_TableInfo *const table_info =
        LCH_TableInfoLoad(LCH_BufferData(table_id), table_definition);
    if (table_info == NULL) {
      LCH_ListDestroy(table_ids);
      LCH_InstanceDestroy(instance);
      LCH_JsonDestroy(config);
      return NULL;
    }

    if (!LCH_ListAppend(instance->tables, table_info, LCH_TableInfoDestroy)) {
      LCH_TableInfoDestroy(table_info);
      LCH_ListDestroy(table_ids);
      LCH_InstanceDestroy(instance);
      LCH_JsonDestroy(config);
      return NULL;
    }
  }
  LCH_ListDestroy(table_ids);
  LCH_JsonDestroy(config);

  return instance;
}

const LCH_TableInfo *LCH_InstanceGetTable(const LCH_Instance *const self,
                                          const char *const table_id) {
  assert(self != NULL);
  assert(self->tables != NULL);
  assert(table_id != NULL);

  const size_t num_tables = LCH_ListLength(self->tables);
  for (size_t i = 0; i < num_tables; i++) {
    const LCH_TableInfo *const table_def =
        (LCH_TableInfo *)LCH_ListGet(self->tables, i);
    assert(table_def != NULL);

    if (LCH_StringEqual(LCH_TableInfoGetIdentifier(table_def), table_id)) {
      return table_def;
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

size_t LCH_InstanceGetMaxChainLength(const LCH_Instance *const instance) {
  assert(instance != NULL);
  return instance->max_chain_length;
}

bool LCH_InstancePrettyPrint(const LCH_Instance *const instance) {
  assert(instance != NULL);
  return instance->pretty_print;
}
