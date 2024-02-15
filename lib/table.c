#include "table.h"

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "definitions.h"
#include "json.h"
#include "list.h"
#include "utils.h"

struct LCH_TableInfo {
  char *identifier;
  LCH_List *primary_fields;
  LCH_List *subsidiary_fields;

  void *source_dlib_handle;
  char *source_locator;

  void *destination_dlib_handle;
  char *destination_locator;

  LCH_LoadTableCallbackFn load_callback;
  LCH_BeginTxCallbackFn begin_tx_callback;
  LCH_EndTxCallbackFn end_tx_callback;
  LCH_InsertCallbackFn insert_callback;
  LCH_DeleteCallbackFn delete_callback;
  LCH_UpdateCallbackFn update_callback;
};

void LCH_TableInfoDestroy(void *const _info) {
  LCH_TableInfo *const info = (LCH_TableInfo *)_info;
  assert(info != NULL);
  assert(info->identifier != NULL);
  assert(info->primary_fields != NULL);
  assert(info->subsidiary_fields != NULL);

  free(info->identifier);
  LCH_ListDestroy(info->primary_fields);
  LCH_ListDestroy(info->subsidiary_fields);

  if (dlclose(info->source_dlib_handle) == -1) {
    LCH_LOG_ERROR("Failed to release reference to dynamic library");
  }
  free(info->source_locator);

  if (info->destination_dlib_handle != NULL &&
      dlclose(info->destination_dlib_handle) == -1) {
    LCH_LOG_ERROR("Failed to release reference to dynamic library");
  }
  free(info->destination_locator);

  free(info);
}

LCH_TableInfo *LCH_TableInfoLoad(const char *const identifer,
                                 const LCH_Json *const definition) {
  assert(identifer != NULL);
  assert(definition != NULL);
  assert(LCH_JsonGetType(definition) == LCH_JSON_TYPE_OBJECT);

  LCH_TableInfo *const info =
      (LCH_TableInfo *)LCH_Allocate(sizeof(LCH_TableInfo));
  if (info == NULL) {
    return NULL;
  }

  info->identifier = LCH_StringDuplicate(identifer);
  if (info->identifier == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const LCH_Json *array = LCH_JsonObjectGetArray(definition, "primary_fields");
  if (array == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }
  info->primary_fields = LCH_ListCreate();
  size_t length = LCH_JsonArrayLength(array);
  for (size_t i = 0; i < length; i++) {
    const char *const str = LCH_JsonArrayGetString(array, i);
    char *const dup = LCH_StringDuplicate(str);
    if (dup == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    if (!LCH_ListAppend(info->primary_fields, dup, free)) {
      free(dup);
      LCH_TableInfoDestroy(info);
      return NULL;
    }
  }
  LCH_ListSort(info->primary_fields,
               (int (*)(const void *, const void *))strcmp);

  array = LCH_JsonObjectGetArray(definition, "subsidiary_fields");
  if (array == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }
  info->subsidiary_fields = LCH_ListCreate();
  length = LCH_JsonArrayLength(array);
  for (size_t i = 0; i < length; i++) {
    const char *const str = LCH_JsonArrayGetString(array, i);
    char *const dup = LCH_StringDuplicate(str);
    if (dup == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    if (!LCH_ListAppend(info->subsidiary_fields, dup, free)) {
      free(dup);
      LCH_TableInfoDestroy(info);
      return NULL;
    }
  }
  LCH_ListSort(info->subsidiary_fields,
               (int (*)(const void *, const void *))strcmp);

  LCH_LOG_VERBOSE("Loading callback functions for table '%s'", identifer);

  const LCH_Json *const source = LCH_JsonObjectGetObject(definition, "source");
  if (source == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const source_locator = LCH_JsonObjectGetString(source, "locator");
  info->source_locator = LCH_StringDuplicate(source_locator);
  if (info->source_locator == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const source_dlib_path =
      LCH_JsonObjectGetString(source, "callbacks");
  if (source_dlib_path == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG("Loading dynamic shared library '%s' for source callbacks",
                source_dlib_path);
  info->source_dlib_handle = dlopen(source_dlib_path, RTLD_NOW);
  if (info->source_dlib_handle == NULL) {
    LCH_LOG_ERROR("Failed to load dynamic shared library '%s': %s",
                  source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'load_callback' from dynamic shared library "
      "'%s'",
      source_dlib_path);
  info->load_callback =
      (LCH_LoadTableCallbackFn)dlsym(info->source_dlib_handle, "load_callback");
  if (info->load_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'load_callback' in dynamic shared "
        "library '%s'",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const LCH_Json *const destination =
      LCH_JsonObjectGetObject(definition, "destination");
  if (source == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const destination_locator =
      LCH_JsonObjectGetString(destination, "locator");
  info->destination_locator = LCH_StringDuplicate(destination_locator);
  if (info->destination_locator == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const destination_dlib_path =
      LCH_JsonObjectGetString(destination, "callbacks");
  if (destination_dlib_path == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG("Loading dynamic shared library '%s' for destination callbacks",
                source_dlib_path);
  info->source_dlib_handle = dlopen(source_dlib_path, RTLD_NOW);
  if (info->source_dlib_handle == NULL) {
    LCH_LOG_ERROR(
        "Failed to load dynamic shared library '%s' for destination callbacks: "
        "'%s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'begin_tx_callback' from dynamic shared "
      "library '%s'",
      source_dlib_path);
  info->begin_tx_callback = (LCH_BeginTxCallbackFn)dlsym(
      info->source_dlib_handle, "begin_tx_callback");
  if (info->begin_tx_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'begin_tx_callback' in dynamic "
        "shared library '%s': %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'end_tx_callback' from dynamic shared "
      "library '%s'",
      source_dlib_path);
  info->end_tx_callback =
      (LCH_EndTxCallbackFn)dlsym(info->source_dlib_handle, "end_tx_callback");
  if (info->end_tx_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'end_tx_callback' in dynamic "
        "shared library '%s': %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'insert_callback' from dynamic shared "
      "library '%s'",
      source_dlib_path);
  info->insert_callback =
      (LCH_InsertCallbackFn)dlsym(info->source_dlib_handle, "insert_callback");
  if (info->insert_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'insert_callback' in dynamic "
        "shared library '%s': %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'delete_callback' from dynamic shared "
      "library '%s'",
      source_dlib_path);
  info->delete_callback =
      (LCH_DeleteCallbackFn)dlsym(info->source_dlib_handle, "delete_callback");
  if (info->delete_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'delete_callback' in dynamic "
        "shared library '%s': %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'update_callback' from dynamic shared "
      "library '%s'",
      source_dlib_path);
  info->update_callback =
      (LCH_UpdateCallbackFn)dlsym(info->source_dlib_handle, "update_callback");
  if (info->update_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'update_callback' in dynamic "
        "shared library '%s': %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  return info;
}

const char *LCH_TableInfoGetIdentifier(const LCH_TableInfo *const table_info) {
  assert(table_info != NULL);
  return table_info->identifier;
}

const LCH_List *LCH_TableInfoGetPrimaryFields(
    const LCH_TableInfo *const table_info) {
  assert(table_info != NULL);
  return table_info->primary_fields;
}

const LCH_List *LCH_TableInfoGetSubsidiaryFields(
    const LCH_TableInfo *const table_info) {
  assert(table_info != NULL);
  return table_info->subsidiary_fields;
}

LCH_Json *LCH_TableInfoLoadNewState(const LCH_TableInfo *const table_info) {
  assert(table_info != NULL);

  char ***const str_table =
      table_info->load_callback(table_info->source_locator);
  if (str_table == NULL) {
    return NULL;
  }

  LCH_List *const list_table = LCH_StringArrayTableToStringListTable(str_table);
  LCH_StringArrayTableDestroy(str_table);
  if (list_table == NULL) {
    return NULL;
  }

  LCH_Json *const state = LCH_TableToJsonObject(
      list_table, table_info->primary_fields, table_info->subsidiary_fields);

  LCH_ListDestroy(list_table);
  return state;
}

LCH_Json *LCH_TableInfoLoadOldState(const LCH_TableInfo *const table_info,
                                    const char *const work_dir) {
  assert(table_info != NULL);
  assert(work_dir != NULL);
  assert(table_info->identifier != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                    table_info->identifier)) {
    return NULL;
  }

  if (!LCH_FileExists(path)) {
    LCH_Json *const state = LCH_JsonObjectCreate();
    return state;
  }

  char *const json = LCH_FileRead(path, NULL);
  if (json == NULL) {
    return NULL;
  }

  LCH_Json *const state = LCH_JsonParse(json);
  return state;
}

bool LCH_TableStoreNewState(const LCH_TableInfo *const self,
                            const char *const work_dir,
                            const LCH_Json *const state) {
  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                    self->identifier)) {
    return false;
  }

  char *const json = LCH_JsonCompose(state);
  if (json == NULL) {
    return false;
  }

  if (!LCH_FileWrite(path, json)) {
    free(json);
    return false;
  }
  free(json);
  return true;
}

void LCH_TableDefinitionDestroy(void *self) { free(self); }
