#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "csv.h"
#include "definitions.h"
#include "list.h"
#include "logger.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *filename;
  LCH_List *table;
} CSVconn;

void *LCH_CallbackConnect(const char *const conn_info) {
  CSVconn *const conn = (CSVconn *)malloc(sizeof(CSVconn));
  if (conn == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  conn->filename = LCH_StringDuplicate(conn_info);
  if (conn->filename == NULL) {
    free(conn);
    return NULL;
  }
  conn->table = NULL;

  return conn;
}

void LCH_CallbackDisconnect(void *const _conn) {
  CSVconn *const conn = (CSVconn *)_conn;
  if (conn != NULL) {
    free(conn->filename);
    LCH_ListDestroy(conn->table);
  }
  free(conn);
}

bool LCH_CallbackCreateTable(void *const _conn, const char *const table_name,
                             const char *const *const primary_columns,
                             const char *const *const subsidiary_columns) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->filename != NULL);

  LCH_UNUSED(table_name);  // Intended for database systems.

  if (LCH_IsRegularFile(conn->filename)) {
    LCH_LOG_DEBUG("Skipped creating table '%s': Table already exists",
                  conn->filename);
    return true;
  }

  LCH_List *const header = LCH_StringArrayToStringList(primary_columns);
  if (header == NULL) {
    return false;
  }

  for (size_t i = 0; subsidiary_columns[i] != NULL; i++) {
    const char *const column = subsidiary_columns[i];
    if (!LCH_ListAppendStringDuplicate(header, column)) {
      LCH_ListDestroy(header);
      return false;
    }
  }

  LCH_List *const table = LCH_ListCreate();
  if (table == NULL) {
    LCH_ListDestroy(header);
    return false;
  }

  if (!LCH_ListAppend(table, header, LCH_ListDestroy)) {
    LCH_ListDestroy(table);
    LCH_ListDestroy(header);
    return false;
  }

  if (!LCH_CSVComposeFile(table, conn->filename)) {
    LCH_ListDestroy(table);
    return false;
  }

  char *const str_repr = LCH_StringJoin(header, "', '");
  LCH_LOG_DEBUG("Created table with header: '%s'", str_repr);
  free(str_repr);

  LCH_ListDestroy(table);
  return true;
}

char ***LCH_CallbackGetTable(void *const _conn, const char *const table_name,
                             const char *const *const columns) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->filename != NULL);
  LCH_UNUSED(table_name);  // Intended for database systems.

  LCH_List *const table = LCH_CSVParseFile(conn->filename);
  if (table == NULL) {
    return NULL;
  }

  LCH_LOG_DEBUG("Loaded table '%s' from '%s'", table_name, conn->filename);

  /**
   * TODO: Extract only the fields listed in the columns parameter, and in the
   * same order as they appear (see ticket CFE-4339).
   */
  LCH_UNUSED(columns);

  char ***const result = LCH_StringListTableToStringArrayTable(table);
  if (result == NULL) {
    LCH_ListDestroy(table);
    return NULL;
  }

  LCH_ListDestroy(table);
  return result;
}

bool LCH_CallbackBeginTransaction(void *const _conn) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->filename != NULL);

  LCH_List *const table = LCH_CSVParseFile(conn->filename);
  if (table == NULL) {
    return false;
  }

  LCH_LOG_DEBUG("Loaded table from '%s'", conn->filename);

  conn->table = table;
  return true;
}

bool LCH_CallbackCommitTransaction(void *const _conn) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->filename != NULL);
  assert(conn->table != NULL);

  if (!LCH_CSVComposeFile(conn->table, conn->filename)) {
    LCH_ListDestroy(conn->table);
    conn->table = NULL;
    return false;
  }

  LCH_LOG_DEBUG("Wrote table to '%s'", conn->filename);

  LCH_ListDestroy(conn->table);
  conn->table = NULL;
  return true;
}

bool LCH_CallbackRollbackTransaction(void *const _conn) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);

  LCH_ListDestroy(conn->table);
  conn->table = NULL;

  LCH_LOG_DEBUG("Destroyed table");

  return true;
}

bool LCH_CallbackInsertRecord(void *const _conn, const char *const table_name,
                              const char *const *const columns,
                              const char *const *const values) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->table != NULL);

  LCH_UNUSED(table_name);  // Intended for database systems.
  LCH_UNUSED(columns);     // Intended for database systems.

  LCH_List *const record = LCH_StringArrayToStringList(values);
  if (record == NULL) {
    return false;
  }

  if (!LCH_ListAppend(conn->table, record, LCH_ListDestroy)) {
    LCH_ListDestroy(record);
    return false;
  }

  char *const str_repr = LCH_StringJoin(record, "', '");
  LCH_LOG_DEBUG("Inserted record %zu: '%s'", LCH_ListLength(conn->table),
                str_repr);
  free(str_repr);

  return true;
}

bool LCH_CallbackDeleteRecord(void *const _conn, const char *const table_name,
                              const char *const *const primary_columns,
                              const char *const *const primary_values) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->table != NULL);

  LCH_UNUSED(table_name);       // Intended for database systems.
  LCH_UNUSED(primary_columns);  // Intended for database systems.

  const size_t num_records = LCH_ListLength(conn->table);
  assert(num_records > 0);

  for (size_t i = 1 /* Skip header */; i < num_records; i++) {
    const LCH_List *const record =
        (const LCH_List *)LCH_ListGet(conn->table, i);
    bool found = true;

    for (size_t j = 0; primary_values[j] != NULL; j++) {
      const char *const field = (const char *)LCH_ListGet(record, j);
      const char *const value = primary_values[j];
      assert(value != NULL);

      if (!LCH_StringEqual(field, value)) {
        found = false;
        break;
      }
    }

    if (found) {
      LCH_List *const removed = (LCH_List *)LCH_ListRemove(conn->table, i);
      char *const str_repr = LCH_StringJoin(removed, "', '");
      LCH_ListDestroy(removed);
      LCH_LOG_DEBUG("Deleted record %zu: '%s'", i + 1, str_repr);
      free(str_repr);
      return true;
    }
  }

  return false;
}

bool LCH_CallbackUpdateRecord(void *const _conn, const char *const table_name,
                              const char *const *const primary_columns,
                              const char *const *const primary_values,
                              const char *const *const subsidiary_columns,
                              const char *const *const subsidiary_values) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->table != NULL);

  LCH_UNUSED(table_name);          // Intended for database systems.
  LCH_UNUSED(primary_columns);     // Intended for database systems.
  LCH_UNUSED(subsidiary_columns);  // Intended for database systems.

  const size_t num_records = LCH_ListLength(conn->table);
  assert(num_records > 0);

  for (size_t i = 1 /* Skip header */; i < num_records; i++) {
    LCH_List *const record = (LCH_List *)LCH_ListGet(conn->table, i);
    bool found = true;

    size_t j;
    for (j = 0; primary_values[j] != NULL; j++) {
      const char *const field = (const char *)LCH_ListGet(record, j);
      const char *const value = primary_values[j];

      if (!LCH_StringEqual(field, value)) {
        found = false;
        break;
      }
    }

    if (found) {
      for (size_t k = 0; subsidiary_values[k] != NULL; k++) {
        char *const value = LCH_StringDuplicate(subsidiary_values[k]);
        if (value == NULL) {
          return false;
        }
        LCH_ListSet(record, j + k, value, free);
      }

      char *const str_repr = LCH_StringJoin(record, "', '");
      LCH_LOG_DEBUG("Updated record %zu: '%s'", i + 1, str_repr);
      free(str_repr);
      return true;
    }
  }

  return false;
}

#ifdef __cplusplus
}
#endif
