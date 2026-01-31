#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "buffer.h"
#include "csv.h"
#include "definitions.h"
#include "files.h"
#include "logger.h"
#include "string_lib.h"
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
                             const LCH_List *const primary_columns,
                             const LCH_List *const subsidiary_columns) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->filename != NULL);
  assert(primary_columns != NULL);
  assert(subsidiary_columns != NULL);

  if (LCH_FileIsRegular(conn->filename)) {
    LCH_LOG_DEBUG("Skipped creating CSV file '%s': Table \"%s\" already exists",
                  conn->filename, table_name);
    return true;
  }

  LCH_List *const header = LCH_ListCopy(
      primary_columns, (LCH_DuplicateFn)LCH_BufferDuplicate, LCH_BufferDestroy);
  if (header == NULL) {
    return false;
  }

  const size_t num_subdidiary = LCH_ListLength(subsidiary_columns);
  for (size_t i = 0; i < num_subdidiary; i++) {
    const LCH_Buffer *const column_name =
        (LCH_Buffer *)LCH_ListGet(subsidiary_columns, i);
    if (!LCH_ListAppendBufferDuplicate(header, column_name)) {
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

  // Print debug info
  LCH_Buffer *csv = NULL;
  if (LCH_CSVComposeRecord(&csv, header)) {
    const char *const str_repr = LCH_BufferData(csv);
    LCH_LOG_DEBUG("Created table with header: \n\t%s", str_repr);
    LCH_BufferDestroy(csv);
  }

  LCH_ListDestroy(table);
  return true;
}

bool LCH_CallbackTruncateTable(void *const _conn, const char *const table_name,
                               const char *const uq_column,
                               const char *const uq_field) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->filename != NULL);
  assert(conn->table != NULL);
  assert(uq_column != NULL);
  assert(uq_field != NULL);

  size_t num_records = LCH_ListLength(conn->table);
  assert(num_records > 0);

  const LCH_List *const table_header = (LCH_List *)LCH_ListGet(conn->table, 0);
  const LCH_Buffer uk_col_key = LCH_BufferStaticFromString(uq_column);
  const size_t uq_col_idx = LCH_ListIndex(table_header, &uk_col_key,
                                          (LCH_CompareFn)LCH_BufferCompare);

  if (uq_col_idx >= LCH_ListLength(table_header)) {
    LCH_LOG_ERROR(
        "Missing field name \"%s\" for unique host identifier "
        "in table header of table '%s'",
        uq_column, table_name);
    return false;
  }

  for (size_t i = 1; i < num_records;) {
    const LCH_List *const record = (LCH_List *)LCH_ListGet(conn->table, i);
    const LCH_Buffer *const field =
        (LCH_Buffer *)LCH_ListGet(record, uq_col_idx);

    const LCH_Buffer uk_field_key = LCH_BufferStaticFromString(uq_field);
    if (LCH_BufferEqual(&uk_field_key, field)) {
      // Records with the unqiue host identifier are to be removed
      LCH_LOG_DEBUG(
          "Deleting record %zu form table \"%s\" because unique host "
          "identifier \"%s\" is '%s' ('%s' == '%s')",
          i, table_name, uq_column, uq_field, uq_field, LCH_BufferData(field));
      LCH_List *const removed = (LCH_List *)LCH_ListRemove(conn->table, i);

      LCH_Buffer *str_repr = NULL;
      if (LCH_CSVComposeRecord(&str_repr, removed)) {
        LCH_LOG_DEBUG("Deleted record contained: %s", LCH_BufferData(str_repr));
        LCH_BufferDestroy(str_repr);
      }

      LCH_ListDestroy(removed);
      num_records -= 1;
    } else {
      i += 1;
    }
  }

  return true;
}

LCH_List *LCH_CallbackGetTable(void *const _conn, const char *const table_name,
                               LCH_UNUSED const LCH_List *const columns) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->filename != NULL);

  LCH_List *const table = LCH_CSVParseFile(conn->filename);
  if (table == NULL) {
    return NULL;
  }
  LCH_LOG_DEBUG("Loaded table \"%s\" from '%s'", table_name, conn->filename);

  /**
   * TODO: Extract only the fields listed in the columns parameter, and in the
   * same order as they appear (see ticket CFE-4339).
   */

  return table;
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

bool LCH_CallbackInsertRecord(void *const _conn,
                              LCH_UNUSED const char *const table_name,
                              LCH_UNUSED const LCH_List *const columns,
                              const LCH_List *const values) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->table != NULL);

  LCH_List *const record = LCH_ListCopy(
      values, (LCH_DuplicateFn)LCH_BufferDuplicate, LCH_BufferDestroy);
  if (record == NULL) {
    return false;
  }

  if (!LCH_ListAppend(conn->table, record, LCH_ListDestroy)) {
    LCH_ListDestroy(record);
    return false;
  }

  LCH_Buffer *str_repr = NULL;
  if (LCH_CSVComposeRecord(&str_repr, record)) {
    LCH_LOG_DEBUG("Inserted record %zu: '%s'", LCH_ListLength(conn->table) - 1,
                  LCH_BufferData(str_repr));
  } else {
    LCH_LOG_DEBUG("Inserted record %zu", LCH_ListLength(conn->table) - 1);
  }
  LCH_BufferDestroy(str_repr);
  return true;
}

bool LCH_CallbackDeleteRecord(void *const _conn,
                              LCH_UNUSED const char *const table_name,
                              LCH_UNUSED const LCH_List *const primary_columns,
                              const LCH_List *const primary_values) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->table != NULL);

  const size_t num_records = LCH_ListLength(conn->table);
  assert(num_records > 0);

  for (size_t i = 1 /* Skip header */; i < num_records; i++) {
    const LCH_List *const record =
        (const LCH_List *)LCH_ListGet(conn->table, i);
    bool found = true;

    const size_t num_primary = LCH_ListLength(primary_values);
    for (size_t j = 0; j < num_primary; j++) {
      const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, j);
      const LCH_Buffer *const value =
          (LCH_Buffer *)LCH_ListGet(primary_values, j);
      assert(value != NULL);

      if (!LCH_BufferEqual(field, value)) {
        found = false;
        break;
      }
    }

    if (found) {
      LCH_List *const removed = (LCH_List *)LCH_ListRemove(conn->table, i);
      LCH_Buffer *str_repr = NULL;
      if (LCH_CSVComposeRecord(&str_repr, removed)) {
        LCH_LOG_DEBUG("Deleted record %zu: '%s'", i + 1,
                      LCH_BufferData(str_repr));
      } else {
        LCH_LOG_DEBUG("Deleted record %zu", i + 1);
      }
      LCH_BufferDestroy(str_repr);
      LCH_ListDestroy(removed);
      return true;
    }
  }

  return false;
}

bool LCH_CallbackUpdateRecord(
    void *const _conn, LCH_UNUSED const char *const table_name,
    LCH_UNUSED const LCH_List *const primary_columns,
    const LCH_List *const primary_values,
    LCH_UNUSED const LCH_List *const subsidiary_columns,
    const LCH_List *const subsidiary_values) {
  CSVconn *const conn = (CSVconn *)_conn;
  assert(conn != NULL);
  assert(conn->table != NULL);

  const size_t num_records = LCH_ListLength(conn->table);
  assert(num_records > 0);

  for (size_t i = 1 /* Skip header */; i < num_records; i++) {
    LCH_List *const record = (LCH_List *)LCH_ListGet(conn->table, i);
    bool found = true;

    size_t j;
    const size_t num_primary = LCH_ListLength(primary_values);
    for (j = 0; j < num_primary; j++) {
      const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, j);
      const LCH_Buffer *const value =
          (LCH_Buffer *)LCH_ListGet(primary_values, j);

      if (!LCH_BufferEqual(field, value)) {
        found = false;
        break;
      }
    }

    if (found) {
      const size_t num_subsidiary = LCH_ListLength(subsidiary_values);
      for (size_t k = 0; k < num_subsidiary; k++) {
        const LCH_Buffer *const value =
            (LCH_Buffer *)LCH_ListGet(subsidiary_values, k);
        LCH_Buffer *const duplicate = LCH_BufferDuplicate(value);
        if (duplicate == NULL) {
          return false;
        }
        LCH_ListSet(record, j + k, duplicate, LCH_BufferDestroy);
      }

      LCH_Buffer *str_repr = NULL;
      if (LCH_CSVComposeRecord(&str_repr, record)) {
        LCH_LOG_DEBUG("Updated record %zu: '%s'", i + 1,
                      LCH_BufferData(str_repr));
      } else {
        LCH_LOG_DEBUG("Updated record %zu", i + 1);
      }
      LCH_BufferDestroy(str_repr);
      return true;
    }
  }

  return false;
}

#ifdef __cplusplus
}
#endif
