#include "table.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "csv.h"
#include "files.h"
#include "list.h"
#include "logger.h"
#include "module.h"
#include "string_lib.h"
#include "utils.h"

typedef void *(*LCH_CallbackConnect)(const char *conn_info);
typedef void (*LCH_CallbackDisconnect)(void *conn);
typedef bool (*LCH_CallbackCreateTable)(void *conn, const char *table_name,
                                        const LCH_List *primary_columns,
                                        const LCH_List *subsidiary_columns);
typedef bool (*LCH_CallbackTruncateTable)(void *conn, const char *table_name,
                                          const char *const field,
                                          const char *const value);
typedef LCH_List *(*LCH_CallbackGetTable)(void *conn, const char *table_name,
                                          const LCH_List *columns);
typedef bool (*LCH_CallbackBeginTransaction)(void *conn);
typedef bool (*LCH_CallbackCommitTransaction)(void *conn);
typedef bool (*LCH_CallbackRollbackTransaction)(void *conn);
typedef bool (*LCH_CallbackInsertRecord)(void *conn, const char *table_name,
                                         const LCH_List *columns,
                                         const LCH_List *values);
typedef bool (*LCH_CallbackDeleteRecord)(void *conn, const char *table_name,
                                         const LCH_List *columns,
                                         const LCH_List *values);
typedef bool (*LCH_CallbackUpdateRecord)(void *conn, const char *table_name,
                                         const LCH_List *primary_columns,
                                         const LCH_List *primary_values,
                                         const LCH_List *subsidiary_columns,
                                         const LCH_List *subsidiary_values);

struct LCH_TableInfo {
  char *identifier;
  LCH_List *all_fields;
  LCH_List *primary_fields;
  LCH_List *subsidiary_fields;
  bool merge_blocks;

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
  LCH_CallbackTruncateTable dst_truncate_table;
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

  LCH_ListDestroy(info->all_fields);
  LCH_ListDestroy(info->primary_fields);
  LCH_ListDestroy(info->subsidiary_fields);

  free(info->src_table_name);
  free(info->src_params);
  free(info->src_schema);
  LCH_ModuleDestroy(info->src_dlib_handle);

  free(info->dst_table_name);
  free(info->dst_params);
  free(info->dst_schema);
  LCH_ModuleDestroy(info->dst_dlib_handle);

  free(info);
}

LCH_TableInfo *LCH_TableInfoLoad(const char *const identifer,
                                 const LCH_Json *const definition) {
  assert(identifer != NULL);
  assert(definition != NULL);
  assert(LCH_JsonGetType(definition) == LCH_JSON_TYPE_OBJECT);

  LCH_TableInfo *const info = (LCH_TableInfo *)malloc(sizeof(LCH_TableInfo));
  if (info == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  memset(info, 0, sizeof(LCH_TableInfo));

  info->identifier = LCH_StringDuplicate(identifer);
  if (info->identifier == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->all_fields = LCH_ListCreate();
  if (info->all_fields == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->primary_fields = LCH_ListCreate();
  if (info->primary_fields == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->subsidiary_fields = LCH_ListCreate();
  if (info->subsidiary_fields == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->merge_blocks = true;
  {
    const LCH_Buffer key = LCH_BufferStaticFromString("merge_blocks");
    if (LCH_JsonObjectHasKey(definition, &key)) {
      info->merge_blocks = LCH_JsonObjectChildIsTrue(definition, &key);
    }
  }

  const LCH_Buffer primary_fields_key =
      LCH_BufferStaticFromString("primary_fields");
  const LCH_Json *const primary_array =
      LCH_JsonObjectGetArray(definition, &primary_fields_key);
  if (primary_array == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const LCH_Buffer subsidiary_fields_key =
      LCH_BufferStaticFromString("subsidiary_fields");
  const LCH_Json *const subsidiary_array =
      LCH_JsonObjectGetArray(definition, &subsidiary_fields_key);
  if (subsidiary_array == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  size_t length = LCH_JsonArrayLength(primary_array);
  for (size_t i = 0; i < length; i++) {
    const LCH_Buffer *const field = LCH_JsonArrayGetString(primary_array, i);
    if (field == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    if (!LCH_ListAppendBufferDuplicate(info->all_fields, field)) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    if (!LCH_ListAppendBufferDuplicate(info->primary_fields, field)) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
  }

  length = LCH_JsonArrayLength(subsidiary_array);
  for (size_t i = 0; i < length; i++) {
    const LCH_Buffer *const field = LCH_JsonArrayGetString(subsidiary_array, i);
    if (field == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    if (!LCH_ListAppendBufferDuplicate(info->all_fields, field)) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    if (!LCH_ListAppendBufferDuplicate(info->subsidiary_fields, field)) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
  }

  LCH_LOG_VERBOSE("Loading callback functions for table '%s'", identifer);
  const LCH_Buffer params_key = LCH_BufferStaticFromString("params");
  const LCH_Buffer schema_key = LCH_BufferStaticFromString("schema");
  const LCH_Buffer table_name_key = LCH_BufferStaticFromString("table_name");
  const LCH_Buffer callbacks_key = LCH_BufferStaticFromString("callbacks");

  {
    const LCH_Buffer source_key = LCH_BufferStaticFromString("source");
    const LCH_Json *const src =
        LCH_JsonObjectGetObject(definition, &source_key);
    if (src == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
    const LCH_Buffer *const params = LCH_JsonObjectGetString(src, &params_key);
    if (params == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
    info->src_params = LCH_StringDuplicate(LCH_BufferData(params));
    if (info->src_params == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const LCH_Buffer *const schema = LCH_JsonObjectGetString(src, &schema_key);
    if (schema == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
    info->src_schema = LCH_StringDuplicate(LCH_BufferData(schema));
    if (info->src_schema == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const LCH_Buffer *const table_name =
        LCH_JsonObjectGetString(src, &table_name_key);
    if (table_name == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
    info->src_table_name = LCH_StringDuplicate(LCH_BufferData(table_name));
    if (info->src_table_name == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    const LCH_Buffer *const callbacks =
        LCH_JsonObjectGetString(src, &callbacks_key);
    if (callbacks == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
    const char *const dlib_path = LCH_BufferData(callbacks);
    if (dlib_path == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    info->src_dlib_handle = LCH_ModuleLoad(dlib_path);
    if (info->src_dlib_handle == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    info->src_connect = (LCH_CallbackConnect)LCH_ModuleGetSymbol(
        info->src_dlib_handle, "LCH_CallbackConnect");
    if (info->src_connect == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    info->src_disconnect = (LCH_CallbackDisconnect)LCH_ModuleGetSymbol(
        info->src_dlib_handle, "LCH_CallbackDisconnect");
    if (info->src_disconnect == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    info->src_create_table = (LCH_CallbackCreateTable)LCH_ModuleGetSymbol(
        info->src_dlib_handle, "LCH_CallbackCreateTable");
    if (info->src_create_table == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }

    info->src_get_table = (LCH_CallbackGetTable)LCH_ModuleGetSymbol(
        info->src_dlib_handle, "LCH_CallbackGetTable");
    if (info->src_create_table == NULL) {
      LCH_TableInfoDestroy(info);
      return NULL;
    }
  }

  const LCH_Buffer dst_key = LCH_BufferStaticFromString("destination");
  const LCH_Json *const dst = LCH_JsonObjectGetObject(definition, &dst_key);
  if (dst == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const LCH_Buffer *const params = LCH_JsonObjectGetString(dst, &params_key);
  if (params == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }
  info->dst_params = LCH_StringDuplicate(LCH_BufferData(params));
  if (info->dst_params == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const LCH_Buffer *const schema = LCH_JsonObjectGetString(dst, &schema_key);
  if (schema == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }
  info->dst_schema = LCH_StringDuplicate(LCH_BufferData(schema));
  if (info->dst_schema == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const LCH_Buffer *const table_name =
      LCH_JsonObjectGetString(dst, &table_name_key);
  if (table_name == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }
  info->dst_table_name = LCH_StringDuplicate(LCH_BufferData(table_name));
  if (info->dst_table_name == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  const LCH_Buffer *const callbacks =
      LCH_JsonObjectGetString(dst, &callbacks_key);
  const char *const dlib_path = LCH_BufferData(callbacks);
  if (dlib_path == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_dlib_handle = LCH_ModuleLoad(dlib_path);
  if (info->dst_dlib_handle == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_connect = (LCH_CallbackConnect)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackConnect");
  if (info->dst_connect == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_disconnect = (LCH_CallbackDisconnect)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackDisconnect");
  if (info->dst_disconnect == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_create_table = (LCH_CallbackCreateTable)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackCreateTable");
  if (info->dst_create_table == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_truncate_table = (LCH_CallbackTruncateTable)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackTruncateTable");
  if (info->dst_truncate_table == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_begin_tx = (LCH_CallbackBeginTransaction)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackBeginTransaction");
  if (info->dst_begin_tx == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_commit_tx = (LCH_CallbackCommitTransaction)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackCommitTransaction");
  if (info->dst_commit_tx == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_rollback_tx = (LCH_CallbackRollbackTransaction)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackRollbackTransaction");
  if (info->dst_rollback_tx == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_insert_record = (LCH_CallbackInsertRecord)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackInsertRecord");
  if (info->dst_insert_record == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_delete_record = (LCH_CallbackDeleteRecord)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackDeleteRecord");
  if (info->dst_delete_record == NULL) {
    LCH_TableInfoDestroy(info);
    return NULL;
  }

  info->dst_update_record = (LCH_CallbackUpdateRecord)LCH_ModuleGetSymbol(
      info->dst_dlib_handle, "LCH_CallbackUpdateRecord");
  if (info->dst_update_record == NULL) {
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

  if (!table_info->src_create_table(conn, table_info->src_table_name,
                                    table_info->primary_fields,
                                    table_info->subsidiary_fields)) {
    LCH_LOG_ERROR("Failed to create table '%s'", table_info->src_table_name);
    table_info->src_disconnect(conn);
    return NULL;
  }

  LCH_List *table = table_info->src_get_table(conn, table_info->src_table_name,
                                              table_info->all_fields);
  if (table == NULL) {
    table_info->src_disconnect(conn);
    return NULL;
  }

  table_info->src_disconnect(conn);

  LCH_Json *const state = LCH_TableToJsonObject(
      table, table_info->primary_fields, table_info->subsidiary_fields);
  LCH_ListDestroy(table);
  return state;
}

LCH_Json *LCH_TableInfoLoadOldState(const LCH_TableInfo *const table_info,
                                    const char *const work_dir) {
  assert(table_info != NULL);
  assert(work_dir != NULL);
  assert(table_info->identifier != NULL);

  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                        table_info->identifier)) {
    return NULL;
  }

  if (!LCH_FileExists(path)) {
    LCH_Json *const state = LCH_JsonObjectCreate();
    return state;
  }

  LCH_Json *const state = LCH_JsonParseFile(path);
  return state;
}

bool LCH_TableStoreNewState(const LCH_TableInfo *const self,
                            const char *const work_dir, const bool pretty_print,
                            const LCH_Json *const state) {
  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, sizeof(path), 3, work_dir, "snapshot",
                        self->identifier)) {
    return false;
  }

  return LCH_JsonComposeFile(state, path, pretty_print);
}

static LCH_List *ConcatenateFields(const LCH_List *const left,
                                   const LCH_List *const right) {
  LCH_List *const record = LCH_ListCreate();
  if (record == NULL) {
    return NULL;
  }

  size_t length = LCH_ListLength(left);
  for (size_t i = 0; i < length; i++) {
    const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(left, i);
    if (!LCH_ListAppendBufferDuplicate(record, field)) {
      LCH_ListDestroy(record);
      return NULL;
    }
  }

  length = LCH_ListLength(right);
  for (size_t i = 0; i < length; i++) {
    const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(right, i);
    if (!LCH_ListAppendBufferDuplicate(record, field)) {
      LCH_ListDestroy(record);
      return NULL;
    }
  }

  return record;
}

static LCH_List *ParseConcatenateFields(const LCH_Buffer *const left_csv,
                                        const LCH_Buffer *const right_csv) {
  const LCH_Buffer empty_str = LCH_BufferStaticFromString("");
  LCH_List *const left_lst =
      (LCH_BufferEqual(left_csv, &empty_str))
          ? LCH_ListCreate()
          : LCH_CSVParseRecord(LCH_BufferData(left_csv),
                               LCH_BufferLength(left_csv));
  if (left_lst == NULL) {
    return NULL;
  }

  LCH_List *const right_lst = LCH_CSVParseRecord(LCH_BufferData(right_csv),
                                                 LCH_BufferLength(right_csv));
  if (right_lst == NULL) {
    LCH_ListDestroy(left_lst);
    return NULL;
  }

  LCH_List *fields = ConcatenateFields(left_lst, right_lst);
  LCH_ListDestroy(left_lst);
  LCH_ListDestroy(right_lst);
  return fields;
}

static bool TablePatchInserts(const LCH_TableInfo *const table_info,
                              const LCH_List *const all_fields,
                              const char *const host_id,
                              const LCH_Json *const inserts, void *const conn) {
  LCH_List *const keys = LCH_JsonObjectGetKeys(inserts);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);
    assert(key != NULL);

    const LCH_Buffer *const value = LCH_JsonObjectGetString(inserts, key);
    if (value == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_List *const values =
        (LCH_ListLength(table_info->subsidiary_fields) == 0)
            ? LCH_CSVParseRecord(LCH_BufferData(key), LCH_BufferLength(key))
            : ParseConcatenateFields(key, value);
    if (values == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_Buffer *const buffer = LCH_BufferFromString(host_id);
    if (buffer == NULL) {
      LCH_ListDestroy(values);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!LCH_ListInsert(values, 0, buffer, LCH_BufferDestroy)) {
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(values);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!table_info->dst_insert_record(conn, table_info->dst_table_name,
                                       all_fields, values)) {
      LCH_ListDestroy(values);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_ListDestroy(values);
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool TablePatchDeletes(const LCH_TableInfo *const table_info,
                              const LCH_List *const primary_fields,
                              const char *const host_id,
                              const LCH_Json *const deletes, void *const conn) {
  LCH_List *const keys = LCH_JsonObjectGetKeys(deletes);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);
    assert(key != NULL);

    LCH_List *const primary_values =
        LCH_CSVParseRecord(LCH_BufferData(key), LCH_BufferLength(key));
    if (primary_values == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_Buffer *const buffer = LCH_BufferFromString(host_id);
    if (buffer == NULL) {
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!LCH_ListInsert(primary_values, 0, buffer, LCH_BufferDestroy)) {
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!table_info->dst_delete_record(conn, table_info->dst_table_name,
                                       primary_fields, primary_values)) {
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_ListDestroy(primary_values);
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool TablePatchUpdates(const LCH_TableInfo *const table_info,
                              const LCH_List *primary_fields,
                              const char *const host_value,
                              const LCH_Json *const updates, void *const conn) {
  LCH_List *const keys = LCH_JsonObjectGetKeys(updates);
  if (keys == NULL) {
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);
    assert(key != NULL);

    LCH_List *primary_values =
        LCH_CSVParseRecord(LCH_BufferData(key), LCH_BufferLength(key));
    if (primary_values == NULL) {
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_Buffer *const buffer = LCH_BufferFromString(host_value);
    if (buffer == NULL) {
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!LCH_ListInsert(primary_values, 0, buffer, LCH_BufferDestroy)) {
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    const LCH_Buffer *const value = LCH_JsonObjectGetString(updates, key);
    if (value == NULL) {
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_List *subsidiary_values =
        LCH_CSVParseRecord(LCH_BufferData(value), LCH_BufferLength(value));
    if (subsidiary_values == NULL) {
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    if (!table_info->dst_update_record(
            conn, table_info->dst_table_name, primary_fields, primary_values,
            table_info->subsidiary_fields, subsidiary_values)) {
      LCH_ListDestroy(subsidiary_values);
      LCH_ListDestroy(primary_values);
      LCH_ListDestroy(keys);
      return false;
    }

    LCH_ListDestroy(subsidiary_values);
    LCH_ListDestroy(primary_values);
  }

  LCH_ListDestroy(keys);
  return true;
}

bool LCH_TablePatch(const LCH_TableInfo *const table_info,
                    const char *const type, const char *const field,
                    const char *const value, const LCH_Json *const inserts,
                    const LCH_Json *const deletes,
                    const LCH_Json *const updates) {
  assert(table_info != NULL);
  assert(type != NULL);
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

  LCH_List *const primary_fields =
      LCH_ListCopy(table_info->primary_fields,
                   (LCH_DuplicateFn)LCH_BufferDuplicate, LCH_BufferDestroy);
  if (primary_fields == NULL) {
    table_info->dst_disconnect(conn);
    return false;
  }

  {
    LCH_Buffer *const buffer = LCH_BufferFromString(field);
    if (buffer == NULL) {
      table_info->dst_disconnect(conn);
      LCH_ListDestroy(primary_fields);
      return false;
    }

    if (!LCH_ListInsert(primary_fields, 0, buffer, LCH_BufferDestroy)) {
      table_info->dst_disconnect(conn);
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(primary_fields);
      return false;
    }
  }

  if (!table_info->dst_create_table(conn, table_info->dst_table_name,
                                    primary_fields,
                                    table_info->subsidiary_fields)) {
    LCH_LOG_ERROR("Failed to create table '%s'", table_info->dst_table_name);
    table_info->dst_disconnect(conn);
    LCH_ListDestroy(primary_fields);
    return false;
  }

  if (!table_info->dst_begin_tx(conn)) {
    LCH_LOG_ERROR("Failed to begin transaction");
    table_info->dst_disconnect(conn);
    LCH_ListDestroy(primary_fields);
    return false;
  }

  if (LCH_StringEqual(type, "rebase")) {
    LCH_LOG_INFO("Patch type is 'rebase': Truncating table '%s'",
                 table_info->dst_table_name);
    if (!table_info->dst_truncate_table(conn, table_info->dst_table_name, field,
                                        value)) {
      LCH_LOG_ERROR("Failed to truncate table");
      table_info->dst_disconnect(conn);
      LCH_ListDestroy(primary_fields);
      return false;
    }
  }

  if (!TablePatchDeletes(table_info, primary_fields, value, deletes, conn)) {
    LCH_LOG_INFO("Performing rollback of transactions for table '%s'",
                 table_info->dst_table_name);
    if (!table_info->dst_rollback_tx(conn)) {
      LCH_LOG_ERROR("Failed to rollback transactions");
    }
    table_info->dst_disconnect(conn);
    LCH_ListDestroy(primary_fields);
    return false;
  }

  if (!TablePatchUpdates(table_info, primary_fields, value, updates, conn)) {
    LCH_LOG_INFO("Performing rollback of transactions for table '%s'",
                 table_info->dst_table_name);
    if (!table_info->dst_rollback_tx(conn)) {
      LCH_LOG_ERROR("Failed to rollback transactions");
    }
    table_info->dst_disconnect(conn);
    LCH_ListDestroy(primary_fields);
    return false;
  }

  LCH_ListDestroy(primary_fields);

  LCH_List *const all_fields =
      LCH_ListCopy(table_info->all_fields, (LCH_DuplicateFn)LCH_BufferDuplicate,
                   LCH_BufferDestroy);
  if (all_fields == NULL) {
    table_info->dst_disconnect(conn);
    return false;
  }

  LCH_Buffer *const buffer = LCH_BufferFromString(field);
  if (buffer == NULL) {
    table_info->dst_disconnect(conn);
    return false;
  }

  if (!LCH_ListInsert(all_fields, 0, buffer, LCH_BufferDestroy)) {
    table_info->dst_disconnect(conn);
    LCH_BufferDestroy(buffer);
    LCH_ListDestroy(all_fields);
    return false;
  }

  if (!TablePatchInserts(table_info, all_fields, value, inserts, conn)) {
    LCH_LOG_INFO("Performing rollback of transactions for table '%s'",
                 table_info->dst_table_name);
    if (!table_info->dst_rollback_tx(conn)) {
      LCH_LOG_ERROR("Failed to rollback transactions");
    }
    table_info->dst_disconnect(conn);
    LCH_ListDestroy(all_fields);
    return false;
  }

  LCH_ListDestroy(all_fields);

  if (!table_info->dst_commit_tx(conn)) {
    LCH_LOG_ERROR("Failed to commit transaction");
    table_info->dst_disconnect(conn);
    return false;
  }

  table_info->dst_disconnect(conn);
  return true;
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

const LCH_List *LCH_TableInfoGetAllFields(
    const LCH_TableInfo *const table_info) {
  assert(table_info != NULL);
  return table_info->all_fields;
}
