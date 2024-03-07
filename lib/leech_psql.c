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
                             const char *const *const primary_columns,
                             const char *const *const subsidiary_columns) {
  PGconn *const conn = (PGconn *)_conn;
  assert(conn != NULL);

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return NULL;
  }

  {
    const size_t len = strlen(table_name);
    char *const identifer = PQescapeIdentifier(conn, table_name, len);
    if (identifer == NULL) {
      LCH_LOG_ERROR("Failed to escape identifer \"%s\": %s", table_name,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    if (!LCH_BufferPrintFormat(buffer, "CREATE TABLE IF NOT EXISTS %s",
                               identifer)) {
      PQfreemem(identifer);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(identifer);
  }

  for (size_t i = 0; primary_columns[i] != NULL; i++) {
    const char *const column = primary_columns[i];
    const size_t len = strlen(column);
    char *const identifier = PQescapeIdentifier(conn, column, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifier \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const format =
        (i == 0) ? " (%s TEXT NOT NULL," : " %s TEXT NOT NULL,";
    if (!LCH_BufferPrintFormat(buffer, format, identifier)) {
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
    }
    PQfreemem(identifier);
  }

  for (size_t i = 0; subsidiary_columns[i] != NULL; i++) {
    const char *const column = subsidiary_columns[i];
    const size_t len = strlen(column);
    char *const identifier = PQescapeIdentifier(conn, column, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifier \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    if (!LCH_BufferPrintFormat(buffer, " %s TEXT,", identifier)) {
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(identifier);
  }

  for (size_t i = 0; primary_columns[i] != NULL; i++) {
    const char *const column = primary_columns[i];
    const size_t len = strlen(column);
    char *const identifier = PQescapeIdentifier(conn, column, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifier \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const format = (i == 0) ? " PRIMARY KEY(%s" : ", %s";
    if (!LCH_BufferPrintFormat(buffer, format, identifier)) {
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(identifier);
  }

  if (!LCH_BufferPrintFormat(buffer, ") );")) {
    LCH_BufferDestroy(buffer);
    return false;
  }

  char *const query = LCH_BufferToString(buffer);
  LCH_LOG_DEBUG("Executing query: %s", query);

  PGresult *const result = PQexec(conn, query);
  free(query);
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

char ***LCH_CallbackGetTable(void *const _conn, const char *const table_name,
                             const char *const *const columns) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return NULL;
  }

  if (!LCH_BufferPrintFormat(buffer, "SELECT ")) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  for (size_t i = 0; columns[i] != NULL; i++) {
    const char *const column = columns[i];
    const size_t len = strlen(column);

    char *const identifier = PQescapeIdentifier(conn, column, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifer \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return NULL;
    }

    const char *const format = (i == 0) ? "%s" : ", %s";
    if (!LCH_BufferPrintFormat(buffer, format, column)) {
      LCH_BufferDestroy(buffer);
      PQfreemem(identifier);
      return NULL;
    }
    PQfreemem(identifier);
  }

  const size_t len = strlen(table_name);
  char *const identifier = PQescapeIdentifier(conn, table_name, len);
  if (identifier == NULL) {
    LCH_LOG_ERROR("Failed to escape identifer \"%s\": %s",
                  PQerrorMessage(conn));
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  if (!LCH_BufferPrintFormat(buffer, " FROM %s;", identifier)) {
    PQfreemem(identifier);
    LCH_BufferDestroy(buffer);
    return NULL;
  }
  PQfreemem(identifier);

  char *const query = LCH_BufferToString(buffer);
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
    const char *const field = PQfname(result, i);
    if (field == NULL) {
      LCH_LOG_ERROR("Failed to get field name at index %d", i);
      LCH_StringArrayDestroy(table);
      PQclear(result);
      return NULL;
    }

    char *const copy = strdup(field);
    if (copy == NULL) {
      LCH_LOG_ERROR("Failed to duplicate string '%s': %s", field,
                    strerror(errno));
      LCH_StringArrayDestroy(table);
      PQclear(result);
      return NULL;
    }
    header[i] = copy;
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

  static const char *const query = "BEGIN;";
  LCH_LOG_DEBUG("Executing query: %s", query);

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

bool LCH_CallbackCommitTransaction(void *const _conn) {
  PGconn *conn = (PGconn *)_conn;

  static const char *const query = "COMMIT;";
  LCH_LOG_DEBUG("Executing query: %s", query);

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

bool LCH_CallbackRollbackTransaction(void *const _conn) {
  PGconn *conn = (PGconn *)_conn;

  static const char *const query = "ROLLBACK;";
  LCH_LOG_DEBUG("Executing query: %s", query);

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

bool LCH_CallbackInsertRecord(void *const _conn, const char *const table_name,
                              const char *const *const columns,
                              const char *const *const values) {
  PGconn *conn = (PGconn *)_conn;

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return false;
  }

  {
    const size_t len = strlen(table_name);
    char *const identifier = PQescapeIdentifier(conn, table_name, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifer \"%s\": %s", identifier,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    if (!LCH_BufferPrintFormat(buffer, "INSERT INTO %s", identifier)) {
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(identifier);
  }

  for (size_t i = 0; columns[i] != NULL; i++) {
    const char *const column = columns[i];
    const size_t len = strlen(column);
    char *const identifier = PQescapeIdentifier(conn, column, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifier \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const format = (i == 0) ? " (%s" : ", %s";
    if (!LCH_BufferPrintFormat(buffer, format, column)) {
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(identifier);
  }

  for (size_t i = 0; values[i] != NULL; i++) {
    const char *const value = values[i];
    const size_t len = strlen(value);

    char *const literal = PQescapeLiteral(conn, value, len);
    if (literal == NULL) {
      LCH_LOG_ERROR("Failed to escape literal '%s': %s", value,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const format = (i == 0) ? ") VALUES (%s" : ", %s";
    if (!LCH_BufferPrintFormat(buffer, format, literal)) {
      PQfreemem(literal);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(literal);
  }

  if (!LCH_BufferPrintFormat(buffer, ");")) {
    LCH_BufferDestroy(buffer);
    return false;
  }

  char *const query = LCH_BufferToString(buffer);
  LCH_LOG_DEBUG("Executing query: %s", query);

  PGresult *const result = PQexec(conn, query);
  free(query);
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

bool LCH_CallbackDeleteRecord(void *const _conn, const char *const table_name,
                              const char *const *const primary_columns,
                              const char *const *const primary_values) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return false;
  }

  {
    const size_t len = strlen(table_name);
    char *const identifier = PQescapeIdentifier(conn, table_name, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifer \"%s\": %s", table_name,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    if (!LCH_BufferPrintFormat(buffer, "DELETE FROM %s", identifier)) {
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(identifier);
  }

  for (size_t i = 0; primary_columns[i] != NULL && primary_values[i] != NULL;
       i++) {
    const char *const column = primary_columns[i];
    size_t len = strlen(column);
    char *const identifer = PQescapeIdentifier(conn, column, len);
    if (identifer == NULL) {
      LCH_LOG_ERROR("Failed to escape identifier \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const value = primary_values[i];
    len = strlen(value);
    char *const literal = PQescapeLiteral(conn, value, len);
    if (literal == NULL) {
      PQfreemem(identifer);
      LCH_LOG_ERROR("Failed to escape literal '%s': %s", value,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const format = (i == 0) ? " WHERE %s = %s" : " AND %s = %s";
    if (!LCH_BufferPrintFormat(buffer, format, identifer, literal)) {
      PQfreemem(literal);
      PQfreemem(identifer);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(literal);
    PQfreemem(identifer);
  }

  if (!LCH_BufferPrintFormat(buffer, ";")) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  char *const query = LCH_BufferToString(buffer);
  LCH_LOG_DEBUG("Executing query: %s", query);

  PGresult *const result = PQexec(conn, query);
  free(query);
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

bool LCH_CallbackUpdateRecord(void *const _conn, const char *const table_name,
                              const char *const *const primary_columns,
                              const char *const *const primary_values,
                              const char *const *const subsidiary_columns,
                              const char *const *const subsidiary_values) {
  PGconn *const conn = (PGconn *)_conn;

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return false;
  }

  {
    const size_t len = strlen(table_name);
    char *const identifier = PQescapeIdentifier(conn, table_name, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifer \"%s\": %s", table_name,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    if (!LCH_BufferPrintFormat(buffer, "UPDATE %s", identifier)) {
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(identifier);
  }

  for (size_t i = 0;
       subsidiary_columns[i] != NULL && subsidiary_values[i] != NULL; i++) {
    const char *const column = subsidiary_columns[i];
    size_t len = strlen(column);
    char *const identifer = PQescapeIdentifier(conn, column, len);
    if (identifer == NULL) {
      LCH_LOG_ERROR("Failed to escape identifier \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const value = subsidiary_values[i];
    len = strlen(value);
    char *const literal = PQescapeLiteral(conn, value, len);
    if (literal == NULL) {
      LCH_LOG_ERROR("Failed to escape literal '%s': %s", value,
                    PQerrorMessage(conn));
      PQfreemem(identifer);
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const format = (i == 0) ? " SET %s = %s" : ", %s = %s";
    if (!LCH_BufferPrintFormat(buffer, format, identifer, literal)) {
      PQfreemem(literal);
      PQfreemem(identifer);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(literal);
    PQfreemem(identifer);
  }

  for (size_t i = 0; primary_columns[i] != NULL && primary_values[i] != NULL;
       i++) {
    const char *const column = primary_columns[i];
    size_t len = strlen(column);
    char *const identifier = PQescapeIdentifier(conn, column, len);
    if (identifier == NULL) {
      LCH_LOG_ERROR("Failed to escape identifier \"%s\": %s", column,
                    PQerrorMessage(conn));
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const value = primary_values[i];
    len = strlen(value);
    char *const literal = PQescapeLiteral(conn, value, len);
    if (literal == NULL) {
      LCH_LOG_ERROR("Failed to escape literal '%s': %s", value,
                    PQerrorMessage(conn));
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }

    const char *const format = (i == 0) ? " WHERE %s = %s" : " AND %s = %s";
    if (!LCH_BufferPrintFormat(buffer, format, identifier, literal)) {
      PQfreemem(literal);
      PQfreemem(identifier);
      LCH_BufferDestroy(buffer);
      return false;
    }
    PQfreemem(literal);
    PQfreemem(identifier);
  }

  if (!LCH_BufferPrintFormat(buffer, ";")) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  char *const query = LCH_BufferToString(buffer);
  LCH_LOG_DEBUG("Executing query: %s", query);

  PGresult *const result = PQexec(conn, query);
  free(query);
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

#ifdef __cplusplus
}
#endif
