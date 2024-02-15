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

typedef void *(*LCH_CallbackConnect)(const char *conn_info);
typedef void (*LCH_CallbackDisconnect)(void *conn);
typedef bool (*LCH_CallbackCreateTable)(void *conn, const char *table_name,
                                        const char *const *primary_columns,
                                        const char *const *subsidiary_columns);
typedef char ***(*LCH_CallbackGetTable)(void *conn, const char *table_name,
                                        const char *const *columns);
typedef bool (*LCH_CallbackBeginTransaction)(void *conn);
typedef bool (*LCH_CallbackCommitTransaction)(void *conn);
typedef bool (*LCH_CallbackRollbackTransaction)(void *conn);
typedef bool (*LCH_CallbackInsertRecord)(void *conn, const char *table_name,
                                         const char *const *columns,
                                         const char *const *values);
typedef bool (*LCH_CallbackDeleteRecord)(void *conn, const char *table_name,
                                         const char *const *columns,
                                         const char *const *values);
typedef bool (*LCH_CallbackUpdateRecord)(void *conn, const char *table_name,
                                         const char *const *primary_columns,
                                         const char *const *primary_values,
                                         const char *const *subsidiary_columns,
                                         const char *const *subsidiary_values);

struct LCH_TableInfo {
  char *identifier;
  LCH_List *primary_fields;
  LCH_List *subsidiary_fields;

  void *src_dlib_handle;
  char *src_params;
  char *src_schema;
  char *src_table_name;

  void *dst_dlib_handle;
  char *dst_params;
  char *dst_schema;
  char *dst_table_name;

  LCH_CallbackConnect src_connect;
  LCH_CallbackDisconnect src_disconnect;
  LCH_CallbackCreateTable src_create_table;
  LCH_CallbackGetTable src_get_table;

  LCH_CallbackConnect dst_connect;
  LCH_CallbackDisconnect dst_disconnect;
  LCH_CallbackCreateTable dst_create_table;
  LCH_CallbackBeginTransaction dst_begin_tx;
  LCH_CallbackCommitTransaction dst_commit_tx;
  LCH_CallbackRollbackTransaction dst_rollback_tx;
  LCH_CallbackInsertRecord dst_insert_record;
  LCH_CallbackDeleteRecord dst_delete_record;
  LCH_CallbackUpdateRecord dst_update_record;
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

  if (dlclose(info->src_dlib_handle) == -1) {
    LCH_LOG_ERROR("Failed to release reference to dynamic library");
  }
  free(info->src_params);

  if (info->dst_dlib_handle != NULL && dlclose(info->dst_dlib_handle) == -1) {
    LCH_LOG_ERROR("Failed to release reference to dynamic library");
  }
  free(info->dst_params);

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

  LCH_LOG_VERBOSE("Loading callback functions for table '%s'", identifer);

  {
    const LCH_Json *const src = LCH_JsonObjectGetObject(definition, "source");
    if (src == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const char *const params = LCH_JsonObjectGetString(src, "params");
    info->src_params = LCH_StringDuplicate(params);
    if (info->src_params == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const char *const schema = LCH_JsonObjectGetString(src, "schema");
    info->src_schema = LCH_StringDuplicate(schema);
    if (info->src_schema == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const char *const table_name = LCH_JsonObjectGetString(src, "table_name");
    info->src_table_name = LCH_StringDuplicate(table_name);
    if (info->src_table_name == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const char *const dlib_path = LCH_JsonObjectGetString(src, "callbacks");
    if (dlib_path == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    LCH_LOG_DEBUG("Loading dynamic shared library '%s' for source callbacks",
                  dlib_path);
    info->src_dlib_handle = dlopen(dlib_path, RTLD_NOW);
    if (info->src_dlib_handle == NULL) {
      LCH_LOG_ERROR("Failed to load dynamic shared library '%s': %s", dlib_path,
                    dlerror());
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const char *symbol = "LCH_CallbackConnect";
    LCH_LOG_DEBUG(
        "Obtaining address of symbol '%s' from dynamic shared library '%s'",
        symbol, dlib_path);
    info->src_connect =
        (LCH_CallbackConnect)dlsym(info->src_dlib_handle, symbol);
    if (info->src_connect == NULL) {
      LCH_LOG_ERROR(
          "Failed to obtain address of symbol '%s' in dynamic shared library "
          "'%s': %s",
          symbol, dlib_path, dlerror());
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    symbol = "LCH_CallbackDisconnect";
    LCH_LOG_DEBUG(
        "Obtaining address of symbol '%s' from dynamic shared library '%s'",
        symbol, dlib_path);
    info->src_disconnect =
        (LCH_CallbackDisconnect)dlsym(info->src_dlib_handle, symbol);
    if (info->src_disconnect == NULL) {
      LCH_LOG_ERROR(
          "Failed to obtain address of symbol '%s' in dynamic shared library "
          "'%s': %s",
          symbol, dlib_path, dlerror());
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    symbol = "LCH_CallbackCreateTable";
    LCH_LOG_DEBUG(
        "Obtaining address of symbol '%s' from dynamic shared library '%s'",
        symbol, dlib_path);
    info->src_create_table =
        (LCH_CallbackCreateTable)dlsym(info->src_dlib_handle, symbol);
    if (info->src_create_table == NULL) {
      LCH_LOG_ERROR(
          "Failed to obtain address of symbol '%s' in dynamic shared library "
          "'%s': %s",
          symbol, dlib_path, dlerror());
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    symbol = "LCH_CallbackGetTable";
    LCH_LOG_DEBUG(
        "Obtaining address of symbol '%s' from dynamic shared library '%s'",
        symbol, dlib_path);
    info->src_get_table =
        (LCH_CallbackGetTable)dlsym(info->src_dlib_handle, symbol);
    if (info->src_create_table == NULL) {
      LCH_LOG_ERROR(
          "Failed to obtain address of symbol '%s' in dynamic shared library "
          "'%s': %s",
          symbol, dlib_path, dlerror());
      LCH_TableInfoDestroy(info);
      return NULL;
    }
  }

  const LCH_Json *const dst =
      LCH_JsonObjectGetObject(definition, "destination");
  if (dst == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const params = LCH_JsonObjectGetString(dst, "params");
  info->dst_params = LCH_StringDuplicate(params);
  if (info->dst_params == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const schema = LCH_JsonObjectGetString(dst, "schema");
  info->dst_schema = LCH_StringDuplicate(schema);
  if (info->dst_schema == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const table_name = LCH_JsonObjectGetString(dst, "table_name");
  info->dst_table_name = LCH_StringDuplicate(table_name);
  if (info->dst_table_name == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *const dlib_path = LCH_JsonObjectGetString(dst, "callbacks");
  if (dlib_path == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  LCH_LOG_DEBUG("Loading dynamic shared library '%s' for destination callbacks",
                dlib_path);
  info->dst_dlib_handle = dlopen(dlib_path, RTLD_NOW);
  if (info->dst_dlib_handle == NULL) {
    LCH_LOG_ERROR(
        "Failed to load dynamic shared library '%s' for destination callbacks: "
        "%s",
        dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const char *symbol = "LCH_CallbackConnect";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_connect = (LCH_CallbackConnect)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_connect == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackDisconnect";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_disconnect =
      (LCH_CallbackDisconnect)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_disconnect == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackCreateTable";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_create_table =
      (LCH_CallbackCreateTable)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_create_table == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackBeginTransaction";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_begin_tx =
      (LCH_CallbackBeginTransaction)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_begin_tx == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackCommitTransaction";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_commit_tx =
      (LCH_CallbackCommitTransaction)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_commit_tx == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackRollbackTransaction";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_rollback_tx =
      (LCH_CallbackRollbackTransaction)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_rollback_tx == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackInsertRecord";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_insert_record =
      (LCH_CallbackInsertRecord)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_insert_record == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackDeleteRecord";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_delete_record =
      (LCH_CallbackDeleteRecord)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_delete_record == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  symbol = "LCH_CallbackUpdateRecord";
  LCH_LOG_DEBUG(
      "Obtaining address of symbol '%s' from dynamic shared library '%s'",
      symbol, dlib_path);
  info->dst_update_record =
      (LCH_CallbackUpdateRecord)dlsym(info->dst_dlib_handle, symbol);
  if (info->dst_update_record == NULL) {
    LCH_LOG_ERROR(
        "Failed to obtain address of symbol '%s' in dynamic shared library "
        "'%s': %s",
        symbol, dlib_path, dlerror());
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

  void *const conn = table_info->src_connect(table_info->src_params);
  if (conn == NULL) {
    LCH_LOG_ERROR("Failed to connect '%s'", table_info->src_params);
    return NULL;
  }

  char **primary_columns =
      LCH_StringListToStringArray(table_info->primary_fields);
  if (primary_columns == NULL) {
    table_info->src_disconnect(conn);
    return NULL;
  }

  char **subsidiary_columns =
      LCH_StringListToStringArray(table_info->subsidiary_fields);
  if (subsidiary_columns == NULL) {
    LCH_StringArrayDestroy(primary_columns);
    table_info->src_disconnect(conn);
    return NULL;
  }

  if (!table_info->src_create_table(conn, table_info->src_table_name,
                                    (const char *const *)primary_columns,
                                    (const char *const *)subsidiary_columns)) {
    LCH_LOG_ERROR("Failed to create table '%s'", table_info->src_table_name);
    LCH_StringArrayDestroy(subsidiary_columns);
    LCH_StringArrayDestroy(primary_columns);
    table_info->src_disconnect(conn);
    return NULL;
  }

  LCH_List *const list = LCH_ListCreate();
  for (size_t i = 0; primary_columns[i] != NULL; i++) {
    char *const column = primary_columns[i];
    if (!LCH_ListAppend(list, column, NULL)) {
      LCH_ListDestroy(list);
      LCH_StringArrayDestroy(subsidiary_columns);
      LCH_StringArrayDestroy(primary_columns);
      table_info->src_disconnect(conn);
      return NULL;
    }
  }

  for (size_t i = 0; subsidiary_columns[i] != NULL; i++) {
    char *const column = subsidiary_columns[i];
    if (!LCH_ListAppend(list, column, NULL)) {
      LCH_ListDestroy(list);
      LCH_StringArrayDestroy(subsidiary_columns);
      LCH_StringArrayDestroy(primary_columns);
      table_info->src_disconnect(conn);
      return NULL;
    }
  }

  char **columns = LCH_StringListToStringArray(list);
  if (columns == NULL) {
    LCH_ListDestroy(list);
    LCH_StringArrayDestroy(subsidiary_columns);
    LCH_StringArrayDestroy(primary_columns);
    table_info->src_disconnect(conn);
    return NULL;
  }

  LCH_ListDestroy(list);
  LCH_StringArrayDestroy(subsidiary_columns);
  LCH_StringArrayDestroy(primary_columns);

  char ***const str_table = table_info->src_get_table(
      conn, table_info->src_table_name, (const char *const *)columns);
  LCH_StringArrayDestroy(columns);
  if (str_table == NULL) {
    return NULL;
  }

  LCH_List *const list_table = LCH_StringArrayTableToStringListTable(
      (const char *const *const *)str_table);
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
