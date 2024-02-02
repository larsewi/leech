#include "table.h"

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "csv.h"
#include "definitions.h"
#include "json.h"
#include "leech.h"
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

  const char ***(*load_callback)(const void *);  // Connection string / locator
  void *(*begin_tx_callback)(const void *);      // Connection string / locator
  bool (*end_tx_callback)(void *,                // Connection info / object
                          int);                  // Error code
  bool (*insert_callback)(void *,                // Connection info / object
                          const char *,          // Table identifer
                          const char *const *,   // Columns
                          const char *const *);  // Values
  bool (*delete_callback)(void *,                // Connection info / object
                          const char *,          // Table identifer
                          const char *const *,   // Columns
                          const char *const *);  // Values
  bool (*update_callback)(void *,                // Connection info / object
                          const char *,          // Table identifer
                          const char *const *,   // Columns
                          const char *const *);  // Values
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

  LCH_LOG_VERBOSE("Loading callback functions");

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
      "%s",
      source_dlib_path);
  info->load_callback = dlsym(info->source_dlib_handle, "load_callback");
  if (info->load_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'load_callback' in dynamic shared "
        "library %s",
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
        "%s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'begin_tx_callback' from dynamic shared "
      "library %s",
      source_dlib_path);
  info->begin_tx_callback =
      dlsym(info->source_dlib_handle, "begin_tx_callback");
  if (info->begin_tx_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'begin_tx_callback' in dynamic "
        "shared library %s: %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'end_tx_callback' from dynamic shared "
      "library %s",
      source_dlib_path);
  info->end_tx_callback = dlsym(info->source_dlib_handle, "end_tx_callback");
  if (info->end_tx_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'end_tx_callback' in dynamic "
        "shared library %s: %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'insert_callback' from dynamic shared "
      "library %s",
      source_dlib_path);
  info->insert_callback = dlsym(info->source_dlib_handle, "insert_callback");
  if (info->insert_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'insert_callback' in dynamic "
        "shared library %s: %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'delete_callback' from dynamic shared "
      "library %s",
      source_dlib_path);
  info->delete_callback = dlsym(info->source_dlib_handle, "delete_callback");
  if (info->delete_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'delete_callback' in dynamic "
        "shared library %s: %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG(
      "Obtaining address of symbol 'update_callback' from dynamic shared "
      "library %s",
      source_dlib_path);
  info->update_callback = dlsym(info->source_dlib_handle, "update_callback");
  if (info->update_callback == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol 'update_callback' in dynamic "
        "shared library %s: %s",
        source_dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  return info;
}

typedef struct LCH_TableDefinition {
  const char *identifier;
  const char *primary_fields;
  const char *subsidiary_fields;
  const void *read_locator;
  const void *write_locator;
  LCH_List *(*read_callback)(const void *);
  bool (*write_callback)(const void *, const LCH_List *);
  bool (*insert_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*delete_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*update_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
} LCH_TableDefinition;

const char *LCH_TableDefinitionGetIdentifier(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->identifier;
}

const char *LCH_TableDefinitionGetPrimaryFields(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->primary_fields;
}

const char *LCH_TableDefinitionGetSubsidiaryFields(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->subsidiary_fields;
}

LCH_TableDefinition *LCH_TableDefinitionCreate(
    const LCH_TableDefinitionCreateInfo *const create_info) {
  assert(create_info != NULL);
  assert(create_info->identifier != NULL);
  assert(create_info->primary_fields != NULL);
  assert(create_info->read_locator != NULL);
  assert(create_info->write_locator != NULL);
  assert(create_info->read_callback != NULL);
  assert(create_info->write_callback != NULL);
  assert(create_info->insert_callback != NULL);
  assert(create_info->delete_callback != NULL);
  assert(create_info->update_callback != NULL);

  LCH_TableDefinition *definition =
      (LCH_TableDefinition *)calloc(1, sizeof(LCH_TableDefinition));
  if (definition == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  definition->identifier = create_info->identifier;
  definition->primary_fields = create_info->primary_fields;
  definition->subsidiary_fields = create_info->subsidiary_fields;
  definition->read_locator = create_info->read_locator;
  definition->write_locator = create_info->write_locator;
  definition->read_callback = create_info->read_callback;
  definition->write_callback = create_info->write_callback;
  definition->insert_callback = create_info->insert_callback;
  definition->delete_callback = create_info->delete_callback;
  definition->update_callback = create_info->update_callback;

  return definition;
}

LCH_Json *LCH_TableDefinitionLoadNewState(
    const LCH_TableDefinition *const self) {
  LCH_List *const table = self->read_callback(self->read_locator);
  if (table == NULL) {
    return NULL;
  }

  LCH_List *const primary_fields = LCH_CSVParseRecord(self->primary_fields);
  if (primary_fields == NULL) {
    LCH_ListDestroy(table);
    return NULL;
  }
  LCH_ListSort(primary_fields, (int (*)(const void *, const void *))strcmp);

  LCH_List *const subsidiary_fields =
      (self->subsidiary_fields != NULL)
          ? LCH_CSVParseRecord(self->subsidiary_fields)
          : LCH_ListCreate();
  if (subsidiary_fields == NULL) {
    LCH_ListDestroy(primary_fields);
    LCH_ListDestroy(table);
    return NULL;
  }
  LCH_ListSort(subsidiary_fields, (int (*)(const void *, const void *))strcmp);

  LCH_Json *const state =
      LCH_TableToJsonObject(table, primary_fields, subsidiary_fields);
  LCH_ListDestroy(subsidiary_fields);
  LCH_ListDestroy(primary_fields);
  LCH_ListDestroy(table);

  return state;
}

LCH_Json *LCH_TableDefinitionLoadOldState(const LCH_TableDefinition *const self,
                                          const char *const work_dir) {
  assert(self != NULL);
  assert(work_dir != NULL);
  assert(self->identifier != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                    self->identifier)) {
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

bool LCH_TableStoreNewState(const LCH_TableDefinition *const self,
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
