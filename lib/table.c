#include "table.h"

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "csv.h"
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
  char **all_fields;
  char **primary_fields;
  char **subsidiary_fields;

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
  if (info == NULL) {
    return;
  }

  free(info->identifier);

  LCH_StringArrayDestroy(info->all_fields);
  LCH_StringArrayDestroy(info->primary_fields);
  LCH_StringArrayDestroy(info->subsidiary_fields);

  free(info->src_table_name);
  free(info->src_params);
  free(info->src_schema);

  if (info->src_dlib_handle != NULL && dlclose(info->src_dlib_handle) == -1) {
    LCH_LOG_ERROR("Failed to release reference to dynamic library");
  }

  free(info->dst_table_name);
  free(info->dst_params);
  free(info->dst_schema);

  if (info->dst_dlib_handle != NULL && dlclose(info->dst_dlib_handle) == -1) {
    LCH_LOG_ERROR("Failed to release reference to dynamic library");
  }

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

  LCH_List *const all_list = LCH_ListCreate();
  LCH_List *const primary_list = LCH_ListCreate();
  LCH_List *const subsidiary_list = LCH_ListCreate();
  const LCH_Json *const primary_array =
      LCH_JsonObjectGetArray(definition, "primary_fields");
  const LCH_Json *const subsidiary_array =
      LCH_JsonObjectGetArray(definition, "subsidiary_fields");
  bool failure = all_list == NULL || primary_list == NULL ||
                 subsidiary_list == NULL || primary_array == NULL ||
                 subsidiary_array == NULL;

  for (size_t i = 0; !failure && (i < LCH_JsonArrayLength(primary_array));
       i++) {
    const char *const field = LCH_JsonArrayGetString(primary_array, i);
    if (field == NULL) {
      failure = true;
      break;
    }
    if (!LCH_ListAppendStringDuplicate(all_list, field)) {
      failure = true;
      break;
    }
    if (!LCH_ListAppendStringDuplicate(primary_list, field)) {
      failure = true;
      break;
    }
  }

  for (size_t i = 0; !failure && (i < LCH_JsonArrayLength(subsidiary_array));
       i++) {
    const char *const field = LCH_JsonArrayGetString(subsidiary_array, i);
    if (field == NULL) {
      failure = true;
      break;
    }
    if (!LCH_ListAppendStringDuplicate(all_list, field)) {
      failure = true;
      break;
    }
    if (!LCH_ListAppendStringDuplicate(subsidiary_list, field)) {
      failure = true;
      break;
    }
  }

  if (failure) {
    LCH_ListDestroy(all_list);
    LCH_ListDestroy(primary_list);
    LCH_ListDestroy(subsidiary_list);
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->all_fields = LCH_StringListToStringArray(all_list);
  info->primary_fields = LCH_StringListToStringArray(primary_list);
  info->subsidiary_fields = LCH_StringListToStringArray(subsidiary_list);
  LCH_ListDestroy(all_list);
  LCH_ListDestroy(primary_list);
  LCH_ListDestroy(subsidiary_list);
  if (info->all_fields == NULL || info->primary_fields == NULL ||
      info->subsidiary_fields == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
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

LCH_Json *LCH_TableInfoLoadNewState(const LCH_TableInfo *const table_info) {
  assert(table_info != NULL);

  void *const conn = table_info->src_connect(table_info->src_params);
  if (conn == NULL) {
    LCH_LOG_ERROR("Failed to connect '%s'", table_info->src_params);
    return NULL;
  }

  if (!table_info->src_create_table(
          conn, table_info->src_table_name,
          (const char *const *)table_info->primary_fields,
          (const char *const *)table_info->subsidiary_fields)) {
    LCH_LOG_ERROR("Failed to create table '%s'", table_info->src_table_name);
    table_info->src_disconnect(conn);
    return NULL;
  }

  char ***const str_table =
      table_info->src_get_table(conn, table_info->src_table_name,
                                (const char *const *)table_info->all_fields);
  if (str_table == NULL) {
    return NULL;
  }

  table_info->src_disconnect(conn);

  LCH_List *const list_table = LCH_StringArrayTableToStringListTable(
      (const char *const *const *)str_table);
  LCH_StringArrayTableDestroy(str_table);
  if (list_table == NULL) {
    return NULL;
  }

  LCH_Json *const state = LCH_TableToJsonObject(
      list_table, (const char *const *)table_info->primary_fields,
      (const char *const *)table_info->subsidiary_fields);

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
  free(json);
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

static char **ParseFields(const char *const str) {
  assert(str != NULL);

  LCH_List *const list = LCH_CSVParseRecord(str);
  if (list == NULL) {
    return NULL;
  }

  char **fields = LCH_StringListToStringArray(list);
  LCH_ListDestroy(list);
  return fields;
}

static char **ConcatenateFields(const LCH_List *const a,
                                const LCH_List *const b) {
  assert(a != NULL);
  assert(b != NULL);

  LCH_List *const list = LCH_ListCreate();
  if (list == NULL) {
    return NULL;
  }

  size_t length = LCH_ListLength(a);
  for (size_t i = 0; i < length; i++) {
    const char *const field = (const char *)LCH_ListGet(a, i);
    if (!LCH_ListAppendStringDuplicate(list, field)) {
      LCH_ListDestroy(list);
      return NULL;
    }
  }

  length = LCH_ListLength(b);
  for (size_t i = 0; i < length; i++) {
    const char *const field = (const char *)LCH_ListGet(b, i);
    if (!LCH_ListAppendStringDuplicate(list, field)) {
      LCH_ListDestroy(list);
      return NULL;
    }
  }

  char **const result = LCH_StringListToStringArray(list);
  LCH_ListDestroy(list);
  return result;
}

static char **ParseConcatenateFields(const char *const str_a,
                                     const char *const str_b) {
  assert(str_a != NULL);
  assert(str_b != NULL);

  LCH_List *const list_a = (LCH_StringEqual(str_a, ""))
                               ? LCH_ListCreate()
                               : LCH_CSVParseRecord(str_a);
  if (list_a == NULL) {
    return NULL;
  }

  LCH_List *const list_b = LCH_CSVParseRecord(str_b);
  if (str_b == NULL) {
    LCH_ListDestroy(list_a);
    return NULL;
  }

  char **fields = ConcatenateFields(list_a, list_b);
  LCH_ListDestroy(list_a);
  LCH_ListDestroy(list_b);
  return fields;
}

static char **AddHostIdentifier(const char *const field,
                                const char *const *const str_array) {
  const size_t length = LCH_StringArrayLength(str_array);
  char **const result = (char **)calloc(length + 2, sizeof(char *));
  if (result == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: calloc(3): %s", strerror(errno));
    return NULL;
  }

  result[0] = LCH_StringDuplicate(field);
  if (result[0] == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < length; i++) {
    result[i + 1] = LCH_StringDuplicate(str_array[i]);
    if (result[i + 1] == NULL) {
      return NULL;
    }
  }

  return result;
}

static bool TablePatchInserts(const LCH_TableInfo *const table_info,
                              const char *const *const all_fields,
                              const char *const host_id,
                              const LCH_Json *const inserts, void *const conn) {
  LCH_List *const keys = LCH_JsonObjectGetKeys(inserts);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (const char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    const char *const value = LCH_JsonObjectGetString(inserts, key);
    if (value == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    char **values =
        (LCH_StringArrayLength(
             (const char *const *)table_info->subsidiary_fields) == 0)
            ? ParseFields(key)
            : ParseConcatenateFields(key, value);
    if (values == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    char **tmp = AddHostIdentifier(host_id, (const char *const *)values);
    LCH_StringArrayDestroy(values);
    if (tmp == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }
    values = tmp;

    if (!table_info->dst_insert_record(conn, table_info->dst_table_name,
                                       (const char *const *)all_fields,
                                       (const char *const *)values)) {
      LCH_StringArrayDestroy(values);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_StringArrayDestroy(values);
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool TablePatchDeletes(const LCH_TableInfo *const table_info,
                              const char *const *const primary_fields,
                              const char *const host_id,
                              const LCH_Json *const deletes, void *const conn) {
  LCH_List *const keys = LCH_JsonObjectGetKeys(deletes);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (const char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    char **primary_values = ParseFields(key);
    if (primary_values == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    char **tmp =
        AddHostIdentifier(host_id, (const char *const *)primary_values);
    LCH_StringArrayDestroy(primary_values);
    if (tmp == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }
    primary_values = tmp;

    if (!table_info->dst_delete_record(conn, table_info->dst_table_name,
                                       (const char *const *)primary_fields,
                                       (const char *const *)primary_values)) {
      LCH_StringArrayDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_StringArrayDestroy(primary_values);
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool TablePatchUpdates(const LCH_TableInfo *const table_info,
                              const char *const *const primary_fields,
                              const char *const host_value,
                              const LCH_Json *const updates, void *const conn) {
  LCH_List *const keys = LCH_JsonObjectGetKeys(updates);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (const char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    char **primary_values = ParseFields(key);
    if (primary_values == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    char **tmp =
        AddHostIdentifier(host_value, (const char *const *)primary_values);
    LCH_StringArrayDestroy(primary_values);
    if (tmp == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }
    primary_values = tmp;

    const char *const value = LCH_JsonObjectGetString(updates, key);
    if (value == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_LOG_ERROR("WHAAT '%s'", value);

    char **subsidiary_values = ParseFields(value);
    if (subsidiary_values == NULL) {
      LCH_StringArrayDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!table_info->dst_update_record(
            conn, table_info->dst_table_name,
            (const char *const *)primary_fields,
            (const char *const *)primary_values,
            (const char *const *)table_info->subsidiary_fields,
            (const char *const *)subsidiary_values)) {
      LCH_StringArrayDestroy(subsidiary_values);
      LCH_StringArrayDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_StringArrayDestroy(subsidiary_values);
    LCH_StringArrayDestroy(primary_values);
  }

  LCH_ListDestroy(keys);
  return true;
}

bool LCH_TablePatch(const LCH_TableInfo *const table_info,
                    const char *const field, const char *const value,
                    const LCH_Json *const inserts,
                    const LCH_Json *const deletes,
                    const LCH_Json *const updates) {
  assert(table_info != NULL);
  assert(field != NULL);
  assert(value != NULL);
  assert(inserts != NULL);
  assert(deletes != NULL);
  assert(updates != NULL);

  void *const conn = table_info->dst_connect(table_info->dst_params);
  if (conn == NULL) {
    LCH_LOG_ERROR("Failed to connect with parameters '%s'",
                  table_info->dst_params);
    return false;
  }

  char **primary_fields =
      AddHostIdentifier(field, (const char *const *)table_info->primary_fields);
  if (primary_fields == NULL) {
    table_info->dst_disconnect(conn);
    return false;
  }

  if (!table_info->dst_create_table(
          conn, table_info->dst_table_name, (const char *const *)primary_fields,
          (const char *const *)table_info->subsidiary_fields)) {
    LCH_LOG_ERROR("Failed to create table '%s'", table_info->dst_table_name);
    table_info->dst_disconnect(conn);
    LCH_StringArrayDestroy(primary_fields);
    return false;
  }

  if (!table_info->dst_begin_tx(conn)) {
    LCH_LOG_ERROR("Failed to begin transaction");
    table_info->dst_disconnect(conn);
    LCH_StringArrayDestroy(primary_fields);
    return false;
  }

  if (!TablePatchDeletes(table_info, (const char *const *)primary_fields, value,
                         deletes, conn)) {
    LCH_LOG_INFO("Performing rollback of transactions for table '%s'",
                 table_info->dst_table_name);
    if (!table_info->dst_rollback_tx(conn)) {
      LCH_LOG_ERROR("Failed to rollback transactions");
    }
    table_info->dst_disconnect(conn);
    LCH_StringArrayDestroy(primary_fields);
    return false;
  }

  if (!TablePatchUpdates(table_info, (const char *const *)primary_fields, value,
                         updates, conn)) {
    LCH_LOG_INFO("Performing rollback of transactions for table '%s'",
                 table_info->dst_table_name);
    if (!table_info->dst_rollback_tx(conn)) {
      LCH_LOG_ERROR("Failed to rollback transactions");
    }
    table_info->dst_disconnect(conn);
    LCH_StringArrayDestroy(primary_fields);
    return false;
  }

  LCH_StringArrayDestroy(primary_fields);

  char **const all_fields =
      AddHostIdentifier(field, (const char *const *)table_info->all_fields);
  if (all_fields == NULL) {
    table_info->dst_disconnect(conn);
    return false;
  }

  if (!TablePatchInserts(table_info, (const char *const *)all_fields, value,
                         inserts, conn)) {
    LCH_LOG_INFO("Performing rollback of transactions for table '%s'",
                 table_info->dst_table_name);
    if (!table_info->dst_rollback_tx(conn)) {
      LCH_LOG_ERROR("Failed to rollback transactions");
    }
    table_info->dst_disconnect(conn);
    LCH_StringArrayDestroy(all_fields);
    return false;
  }

  LCH_StringArrayDestroy(all_fields);

  if (!table_info->dst_commit_tx(conn)) {
    LCH_LOG_ERROR("Failed to commit transaction");
    table_info->dst_disconnect(conn);
    return false;
  }

  table_info->dst_disconnect(conn);
  return true;
}
