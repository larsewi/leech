#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "definitions.h"
#include "libpq-fe.h"
#include "logger.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

static char *EscapeIdentifier(PGconn *const conn,
                              const LCH_Buffer *const identifier) {
  const char *const data = LCH_BufferData(identifier);
  const size_t length = LCH_BufferLength(identifier);
  char *const escaped = PQescapeIdentifier(conn, data, length);
  if (escaped == NULL) {
    LCH_LOG_ERROR("Failed to escape identifier \"%s\" for SQL query: %s", data,
                  PQerrorMessage(conn));
  }
  return escaped;
}

static char *EscapeLiteral(PGconn *const conn,
                           const LCH_Buffer *const literal) {
  const char *const data = LCH_BufferData(literal);
  const size_t length = LCH_BufferLength(literal);
  char *const escaped = PQescapeLiteral(conn, data, length);
  if (escaped == NULL) {
    LCH_LOG_ERROR("Failed to escape literal '%s' for SQL query: %s", literal,
                  PQerrorMessage(conn));
  }
  return escaped;
}

static bool ExecuteCommand(PGconn *const conn, const char *const query) {
  LCH_LOG_DEBUG("Executing command: %s", query);
  PGresult *const result = PQexec(conn, query);
  if (result == NULL) {
    LCH_LOG_ERROR("Failed to execute query: Likely out of memory");
    return false;
  }

  ExecStatusType status = PQresultStatus(result);
  if (status != PGRES_COMMAND_OK) {
    LCH_LOG_ERROR("Failed to execute query: %s", PQerrorMessage(conn));
    PQclear(result);
    return false;
  }

  PQclear(result);
  return true;
}

void *LCH_CallbackConnect(const char *const conn_info) {
  PGconn *const conn = PQconnectdb(conn_info);
  if (conn == NULL) {
    LCH_LOG_ERROR("Failed to connect to database: Likely out of memory");
    return NULL;
  }

  if (PQstatus(conn) != CONNECTION_OK) {
    LCH_LOG_ERROR("Failed to connect to database: %s", PQerrorMessage(conn));
    PQfinish(conn);
    return NULL;
  }

  LCH_LOG_DEBUG("Connection to database established");
  LCH_LOG_DEBUG("\tPort: %s", PQport(conn));
  LCH_LOG_DEBUG("\tHost: %s", PQhost(conn));
  LCH_LOG_DEBUG("\tName: %s", PQdb(conn));
  LCH_LOG_DEBUG("\tUser: %s", PQuser(conn));

  return conn;
}

void LCH_CallbackDisconnect(void *const _conn) {
  PGconn *const conn = (PGconn *)_conn;
  PQfinish(conn);
}

bool LCH_CallbackCreateTable(void *const _conn, const char *const table_name,
                             const LCH_List *const primary_column_names,
                             const LCH_List *const subsidiary_columns_names) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped =
      EscapeIdentifier(conn, LCH_BufferStaticFromString(table_name));
  if (table_name_escaped == NULL) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  if (!LCH_BufferPrintFormat(query_buffer, "CREATE TABLE IF NOT EXISTS %s",
                             table_name_escaped)) {
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }
  PQfreemem(table_name_escaped);

  const size_t num_primary = LCH_ListLength(primary_column_names);
  for (size_t i = 0; i < num_primary; i++) {
    const LCH_Buffer *const column_name =
        (LCH_Buffer *)LCH_ListGet(primary_column_names, i);
    char *const column_name_escaped = EscapeIdentifier(conn, column_name);
    if (column_name_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format =
        (i == 0) ? " (%s TEXT NOT NULL," : " %s TEXT NOT NULL,";
    if (!LCH_BufferPrintFormat(query_buffer, format, column_name_escaped)) {
      PQfreemem(column_name_escaped);
      LCH_BufferDestroy(query_buffer);
    }
    PQfreemem(column_name_escaped);
  }

  const size_t num_subsidiary = LCH_ListLength(subsidiary_columns_names);
  for (size_t i = 0; i < num_subsidiary; i++) {
    const LCH_Buffer *const column_name =
        (LCH_Buffer *)LCH_ListGet(subsidiary_columns_names, i);
    char *const column_name_escaped = EscapeIdentifier(conn, column_name);
    if (column_name_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    if (!LCH_BufferPrintFormat(query_buffer, " %s TEXT,",
                               column_name_escaped)) {
      PQfreemem(column_name_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(column_name_escaped);
  }

  for (size_t i = 0; i < num_primary; i++) {
    const LCH_Buffer *const column_name =
        (LCH_Buffer *)LCH_ListGet(primary_column_names, i);
    char *const column_name_escaped = EscapeIdentifier(conn, column_name);
    if (column_name_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format = (i == 0) ? " PRIMARY KEY(%s" : ", %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, column_name_escaped)) {
      PQfreemem(column_name_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(column_name_escaped);
  }

  if (!LCH_BufferPrintFormat(query_buffer, ") );")) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  char *const query = LCH_BufferToString(query_buffer);
  const bool success = ExecuteCommand(conn, query);
  free(query);
  return success;
}

bool LCH_CallbackTruncateTable(void *const _conn, const char *const table_name,
                               const char *const column,
                               const char *const value) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped =
      EscapeIdentifier(conn, LCH_BufferStaticFromString(table_name));
  if (table_name_escaped == NULL) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  char *const column_escaped =
      EscapeIdentifier(conn, LCH_BufferStaticFromString(column));
  if (column_escaped == NULL) {
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  char *const value_escaped =
      EscapeLiteral(conn, LCH_BufferStaticFromString(value));
  if (value_escaped == NULL) {
    PQfreemem(column_escaped);
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  if (!LCH_BufferPrintFormat(query_buffer, "DELETE FROM %s WHERE %s = %s;",
                             table_name_escaped, column_escaped,
                             value_escaped)) {
    PQfreemem(value_escaped);
    PQfreemem(column_escaped);
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  PQfreemem(value_escaped);
  PQfreemem(column_escaped);
  PQfreemem(table_name_escaped);

  char *const query = LCH_BufferToString(query_buffer);
  const bool success = ExecuteCommand(conn, query);
  free(query);
  return success;
}

LCH_List *LCH_CallbackGetTable(void *const _conn, const char *const table_name,
                               const LCH_List *const columns) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return NULL;
  }

  if (!LCH_BufferPrintFormat(query_buffer, "SELECT ")) {
    LCH_BufferDestroy(query_buffer);
    return NULL;
  }

  const size_t num_columns = LCH_ListLength(columns);
  for (size_t i = 0; i < num_columns; i++) {
    const LCH_Buffer *const column = (LCH_Buffer *)LCH_ListGet(columns, i);
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return NULL;
    }

    const char *const format = (i == 0) ? "%s" : ", %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, column_escaped)) {
      LCH_BufferDestroy(query_buffer);
      PQfreemem(column_escaped);
      return NULL;
    }
    PQfreemem(column_escaped);
  }

  char *const table_name_escaped =
      EscapeIdentifier(conn, LCH_BufferStaticFromString(table_name));
  if (table_name_escaped == NULL) {
    LCH_BufferDestroy(query_buffer);
    return NULL;
  }

  if (!LCH_BufferPrintFormat(query_buffer, " FROM %s;", table_name_escaped)) {
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return NULL;
  }
  PQfreemem(table_name_escaped);

  char *const query = LCH_BufferToString(query_buffer);
  LCH_LOG_DEBUG("Executing query: %s", query);

  PGresult *result = PQexec(conn, query);
  free(query);
  if (result == NULL) {
    LCH_LOG_ERROR("Failed to execute query: Likely out of memory", query);
    return NULL;
  }

  ExecStatusType status = PQresultStatus(result);
  if (status != PGRES_TUPLES_OK) {
    LCH_LOG_ERROR("Failed to execute query: %s", PQerrorMessage(conn));
    PQclear(result);
    return NULL;
  }

  const int n_rows = PQntuples(result);
  const int n_cols = PQnfields(result);
  LCH_LOG_DEBUG("Query returned %d rows and %d columns", n_rows, n_cols);

  LCH_List *const table = LCH_ListCreate();
  if (table == NULL) {
    PQclear(result);
    return NULL;
  }

  LCH_List *const header = LCH_ListCreate();
  if (table == NULL) {
    LCH_ListDestroy(table);
    PQclear(result);
    return NULL;
  }

  if (!LCH_ListAppend(table, header, LCH_ListDestroy)) {
    LCH_ListDestroy(header);
    LCH_ListDestroy(table);
    PQclear(result);
    return NULL;
  }

  for (int i = 0; i < n_cols; i++) {
    const char *const field_name = PQfname(result, i);
    if (field_name == NULL) {
      LCH_LOG_ERROR("Failed to get field name at index %d", i);
      LCH_ListDestroy(table);
      PQclear(result);
      return NULL;
    }

    LCH_Buffer *const buffer = LCH_BufferFromString(field_name);
    if (buffer == NULL) {
      LCH_ListDestroy(table);
      PQclear(result);
      return NULL;
    }

    if (!LCH_ListAppend(header, buffer, LCH_BufferDestroy)) {
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(table);
      PQclear(result);
      return NULL;
    }
  }

  for (int i = 0; i < n_rows; i++) {
    LCH_List *const record = LCH_ListCreate();
    if (record == NULL) {
      LCH_ListDestroy(table);
      PQclear(result);
      return NULL;
    }

    if (!LCH_ListAppend(table, record, LCH_ListDestroy)) {
      LCH_ListDestroy(record);
      LCH_ListDestroy(table);
      PQclear(result);
      return NULL;
    }

    for (int j = 0; j < n_cols; j++) {
      const char *const value = PQgetvalue(result, i, j);
      if (value == NULL) {
        LCH_LOG_ERROR("Failed to get value at index %d:%d", i, j);
        LCH_ListDestroy(table);
        PQclear(result);
        return NULL;
      }

      LCH_Buffer *const buffer = LCH_BufferFromString(value);
      if (buffer == NULL) {
        LCH_ListDestroy(table);
        PQclear(result);
        return NULL;
      }

      if (!LCH_ListAppend(record, buffer, LCH_BufferDestroy)) {
        LCH_BufferDestroy(buffer);
        LCH_ListDestroy(table);
        PQclear(result);
        return NULL;
      }
    }
  }

  PQclear(result);
  return table;
}

bool LCH_CallbackBeginTransaction(void *const _conn) {
  PGconn *conn = (PGconn *)_conn;
  assert(conn != NULL);

  const bool success = ExecuteCommand(conn, "BEGIN;");
  return success;
}

bool LCH_CallbackCommitTransaction(void *const _conn) {
  PGconn *conn = (PGconn *)_conn;
  assert(conn != NULL);

  const bool success = ExecuteCommand(conn, "COMMIT;");
  return success;
}

bool LCH_CallbackRollbackTransaction(void *const _conn) {
  PGconn *conn = (PGconn *)_conn;
  assert(conn != NULL);

  const bool success = ExecuteCommand(conn, "ROLLBACK;");
  return success;
}

bool LCH_CallbackInsertRecord(void *const _conn, const char *const table_name,
                              const LCH_List *const columns,
                              const LCH_List *const values) {
  PGconn *conn = (PGconn *)_conn;

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped =
      EscapeIdentifier(conn, LCH_BufferStaticFromString(table_name));
  if (table_name_escaped == NULL) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  if (!LCH_BufferPrintFormat(query_buffer, "INSERT INTO %s",
                             table_name_escaped)) {
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }
  PQfreemem(table_name_escaped);

  const size_t num_columns = LCH_ListLength(columns);
  for (size_t i = 0; i < num_columns; i++) {
    const LCH_Buffer *const column = (LCH_Buffer *)LCH_ListGet(columns, i);
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format = (i == 0) ? " (%s" : ", %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, column_escaped)) {
      PQfreemem(column_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(column_escaped);
  }

  const size_t num_values = LCH_ListLength(values);
  for (size_t i = 0; i < num_values; i++) {
    const LCH_Buffer *const value = (LCH_Buffer *)LCH_ListGet(values, i);
    char *const value_escaped = EscapeLiteral(conn, value);
    if (value_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format = (i == 0) ? ") VALUES (%s" : ", %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, value_escaped)) {
      PQfreemem(value_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(value_escaped);
  }

  if (!LCH_BufferPrintFormat(query_buffer, ");")) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  char *const query = LCH_BufferToString(query_buffer);
  const bool success = ExecuteCommand(conn, query);
  free(query);
  return true;
}

bool LCH_CallbackDeleteRecord(void *const _conn, const char *const table_name,
                              const LCH_List *const primary_columns,
                              const LCH_List *const primary_values) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped =
      EscapeIdentifier(conn, LCH_BufferStaticFromString(table_name));
  if (table_name_escaped == NULL) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  if (!LCH_BufferPrintFormat(query_buffer, "DELETE FROM %s",
                             table_name_escaped)) {
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }
  PQfreemem(table_name_escaped);

  const size_t num_columns = LCH_ListLength(primary_columns);
  assert(num_columns == LCH_ListLength(primary_values));
  for (size_t i = 0; i < num_columns; i++) {
    const LCH_Buffer *const column =
        (LCH_Buffer *)LCH_ListGet(primary_columns, i);
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const LCH_Buffer *const value =
        (LCH_Buffer *)LCH_ListGet(primary_values, i);
    char *const value_escaped = EscapeLiteral(conn, value);
    if (value_escaped == NULL) {
      PQfreemem(column_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format = (i == 0) ? " WHERE %s = %s" : " AND %s = %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, column_escaped,
                               value_escaped)) {
      PQfreemem(value_escaped);
      PQfreemem(column_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(value_escaped);
    PQfreemem(column_escaped);
  }

  if (!LCH_BufferPrintFormat(query_buffer, ";")) {
    LCH_BufferDestroy(query_buffer);
    return NULL;
  }

  char *const query = LCH_BufferToString(query_buffer);
  const bool success = ExecuteCommand(conn, query);
  free(query);
  return success;
}

bool LCH_CallbackUpdateRecord(void *const _conn, const char *const table_name,
                              const LCH_List *const primary_columns,
                              const LCH_List *const primary_values,
                              const LCH_List *const subsidiary_columns,
                              const LCH_List *const subsidiary_values) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped =
      EscapeIdentifier(conn, LCH_BufferStaticFromString(table_name));
  if (table_name_escaped == NULL) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  if (!LCH_BufferPrintFormat(query_buffer, "UPDATE %s", table_name_escaped)) {
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }
  PQfreemem(table_name_escaped);

  const size_t num_subsidiary = LCH_ListLength(subsidiary_columns);
  assert(num_subsidiary == LCH_ListLength(subsidiary_values));
  for (size_t i = 0; i < num_subsidiary; i++) {
    const LCH_Buffer *const column =
        (LCH_Buffer *)LCH_ListGet(subsidiary_columns, i);
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const LCH_Buffer *const value =
        (LCH_Buffer *)LCH_ListGet(subsidiary_values, i);
    char *const value_escaped = EscapeLiteral(conn, value);
    if (value_escaped == NULL) {
      PQfreemem(column_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format = (i == 0) ? " SET %s = %s" : ", %s = %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, column_escaped,
                               value_escaped)) {
      PQfreemem(value_escaped);
      PQfreemem(column_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(value_escaped);
    PQfreemem(column_escaped);
  }

  const size_t num_primary = LCH_ListLength(primary_columns);
  assert(num_primary == LCH_ListLength(primary_values));
  for (size_t i = 0; i < num_primary; i++) {
    const LCH_Buffer *const column =
        (LCH_Buffer *)LCH_ListGet(primary_columns, i);
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const LCH_Buffer *const value =
        (LCH_Buffer *)LCH_ListGet(primary_values, i);
    char *const value_escaped = EscapeLiteral(conn, value);
    if (value_escaped == NULL) {
      PQfreemem(column_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format = (i == 0) ? " WHERE %s = %s" : " AND %s = %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, column_escaped,
                               value_escaped)) {
      PQfreemem(value_escaped);
      PQfreemem(column_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(value_escaped);
    PQfreemem(column_escaped);
  }

  if (!LCH_BufferPrintFormat(query_buffer, ";")) {
    LCH_BufferDestroy(query_buffer);
    return NULL;
  }

  char *const query = LCH_BufferToString(query_buffer);
  const bool success = ExecuteCommand(conn, query);
  return success;
}

#ifdef __cplusplus
}
#endif
