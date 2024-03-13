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
                              const char *const identifier) {
  assert(identifier != NULL);

  const size_t len = strlen(identifier);
  char *const escaped = PQescapeIdentifier(conn, identifier, len);
  if (escaped == NULL) {
    LCH_LOG_ERROR("Failed to escape identifier \"%s\" for SQL query: %s",
                  identifier, PQerrorMessage(conn));
  }
  return escaped;
}

static char *EscapeLiteral(PGconn *const conn, const char *const literal) {
  assert(literal != NULL);

  const size_t len = strlen(literal);
  char *const escaped = PQescapeLiteral(conn, literal, len);
  if (escaped == NULL) {
    LCH_LOG_ERROR("Failed to escape literal '%s' for SQL query: %s", literal,
                  PQerrorMessage(conn));
  }
  return escaped;
}

static bool ExecuteCommand(PGconn *const conn, const char *const query) {
  assert(conn != NULL);
  assert(query != NULL);

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
  assert(conn_info != NULL);

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
  assert(conn != NULL);

  PQfinish(conn);
}

bool LCH_CallbackCreateTable(
    void *const _conn, const char *const table_name,
    const char *const *const primary_column_names,
    const char *const *const subsidiary_columns_names) {
  PGconn *const conn = (PGconn *)_conn;
  assert(conn != NULL);
  assert(table_name != NULL);
  assert(primary_column_names != NULL);
  assert(subsidiary_columns_names != NULL);

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return NULL;
  }

  char *const table_name_escaped = EscapeIdentifier(conn, table_name);
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

  for (size_t i = 0; primary_column_names[i] != NULL; i++) {
    const char *const column_name = primary_column_names[i];
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

  for (size_t i = 0; subsidiary_columns_names[i] != NULL; i++) {
    const char *const column_name = subsidiary_columns_names[i];
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

  for (size_t i = 0; primary_column_names[i] != NULL; i++) {
    const char *const column_name = primary_column_names[i];
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
  assert(conn != NULL);
  assert(table_name != NULL);
  assert(column != NULL);
  assert(value != NULL);

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped = EscapeIdentifier(conn, table_name);
  if (table_name_escaped == NULL) {
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  char *const column_escaped = EscapeIdentifier(conn, column);
  if (column_escaped == NULL) {
    PQfreemem(table_name_escaped);
    LCH_BufferDestroy(query_buffer);
    return false;
  }

  char *const value_escaped = EscapeLiteral(conn, value);
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

char ***LCH_CallbackGetTable(void *const _conn, const char *const table_name,
                             const char *const *const columns) {
  PGconn *const conn = (PGconn *)_conn;
  assert(conn != NULL);
  assert(table_name != NULL);
  assert(columns != NULL);

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return NULL;
  }

  if (!LCH_BufferPrintFormat(query_buffer, "SELECT ")) {
    LCH_BufferDestroy(query_buffer);
    return NULL;
  }

  for (size_t i = 0; columns[i] != NULL; i++) {
    const char *const column = columns[i];
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

  char *const table_name_escaped = EscapeIdentifier(conn, table_name);
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

  char ***const table = (char ***)calloc(n_rows + 2, sizeof(char **));
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for table: %s", strerror(errno));
    PQclear(result);
    return NULL;
  }

  char **const header = (char **)calloc(n_cols + 1, sizeof(char *));
  if (header == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for table header: %s",
                  strerror(errno));
    LCH_StringArrayDestroy(table);
    PQclear(result);
    return NULL;
  }
  table[0] = header;

  for (int i = 0; i < n_cols; i++) {
    const char *const field_name = PQfname(result, i);
    if (field_name == NULL) {
      LCH_LOG_ERROR("Failed to get field name at index %d", i);
      LCH_StringArrayDestroy(table);
      PQclear(result);
      return NULL;
    }

    char *const header_copy = strdup(field_name);
    if (header_copy == NULL) {
      LCH_LOG_ERROR("Failed to duplicate string '%s': %s", field_name,
                    strerror(errno));
      LCH_StringArrayDestroy(table);
      PQclear(result);
      return NULL;
    }
    header[i] = header_copy;
  }

  for (int i = 0; i < n_rows; i++) {
    char **const record = (char **)calloc(n_cols + 1, sizeof(char *));
    if (record == NULL) {
      LCH_LOG_ERROR("Failed to allocate memory table record: %s",
                    strerror(errno));
      LCH_StringArrayDestroy(table);
      PQclear(result);
      return NULL;
    }
    table[i + 1] = record;

    for (int j = 0; j < n_cols; j++) {
      const char *const value = PQgetvalue(result, i, j);
      if (value == NULL) {
        LCH_LOG_ERROR("Failed to get value at index %d:%d", i, j);
        LCH_StringArrayDestroy(table);
        PQclear(result);
        return NULL;
      }

      char *const copy = strdup(value);
      if (copy == NULL) {
        LCH_LOG_ERROR("Failed to duplicate string: %s", strerror(errno));
        LCH_StringArrayDestroy(table);
        PQclear(result);
      }
      record[j] = copy;
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
                              const char *const *const columns,
                              const char *const *const values) {
  PGconn *conn = (PGconn *)_conn;
  assert(conn != NULL);
  assert(table_name != NULL);
  assert(columns != NULL);
  assert(values != NULL);

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped = EscapeIdentifier(conn, table_name);
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

  for (size_t i = 0; columns[i] != NULL; i++) {
    const char *const column = columns[i];
    char *const columns_escaped = EscapeIdentifier(conn, column);
    if (columns_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const format = (i == 0) ? " (%s" : ", %s";
    if (!LCH_BufferPrintFormat(query_buffer, format, column)) {
      PQfreemem(columns_escaped);
      LCH_BufferDestroy(query_buffer);
      return false;
    }
    PQfreemem(columns_escaped);
  }

  for (size_t i = 0; values[i] != NULL; i++) {
    const char *const value = values[i];
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
                              const char *const *const primary_columns,
                              const char *const *const primary_values) {
  PGconn *const conn = (PGconn *)_conn;
  assert(conn != NULL);
  assert(table_name != NULL);
  assert(primary_columns != NULL);
  assert(primary_values != NULL);

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped = EscapeIdentifier(conn, table_name);
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

  for (size_t i = 0; primary_columns[i] != NULL && primary_values[i] != NULL;
       i++) {
    const char *const column = primary_columns[i];
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const value = primary_values[i];
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
                              const char *const *const primary_columns,
                              const char *const *const primary_values,
                              const char *const *const subsidiary_columns,
                              const char *const *const subsidiary_values) {
  PGconn *const conn = (PGconn *)_conn;
  assert(conn != NULL);
  assert(primary_columns != NULL);
  assert(primary_values != NULL);
  assert(subsidiary_columns != NULL);
  assert(subsidiary_values != NULL);

  LCH_Buffer *const query_buffer = LCH_BufferCreate();
  if (query_buffer == NULL) {
    return false;
  }

  char *const table_name_escaped = EscapeIdentifier(conn, table_name);
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

  for (size_t i = 0;
       subsidiary_columns[i] != NULL && subsidiary_values[i] != NULL; i++) {
    const char *const column = subsidiary_columns[i];
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const value = subsidiary_values[i];
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

  for (size_t i = 0; primary_columns[i] != NULL && primary_values[i] != NULL;
       i++) {
    const char *const column = primary_columns[i];
    char *const column_escaped = EscapeIdentifier(conn, column);
    if (column_escaped == NULL) {
      LCH_BufferDestroy(query_buffer);
      return false;
    }

    const char *const value = primary_values[i];
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
