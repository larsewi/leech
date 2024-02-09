#include <assert.h>
#include <stdbool.h>

#include "csv.h"
#include "definitions.h"
#include "list.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

char ***load_callback(void *locator) {
  assert(locator != NULL);
  const char *const filename = (const char *)locator;

  LCH_List *const table = LCH_CSVParseFile(filename);
  if (table == NULL) {
    return NULL;
  }

  char ***const str_table = LCH_StringListTableToStringArrayTable(table);
  return str_table;
}

/**
 * TODO: Implement begin_tx_callback.
 */
void *begin_tx_callback(void *locator) {
  UNUSED(locator);
  return NULL;
}

/**
 * TODO: Implement end_tx_callback.
 */
bool end_tx_callback(void *conn, int err) {
  UNUSED(conn);
  UNUSED(err);
  return false;
}

/**
 * TODO: Implement insert_callback.
 */
bool insert_callback(void *conn, const char *tid, const char *const *cols,
                     const char *const *vals) {
  UNUSED(conn);
  UNUSED(tid);
  UNUSED(cols);
  UNUSED(vals);
  return false;
}

/**
 * TODO: Implement delete_callback.
 */
bool delete_callback(void *conn, const char *tid, const char *const *cols,
                     const char *const *vals) {
  UNUSED(conn);
  UNUSED(tid);
  UNUSED(cols);
  UNUSED(vals);
  return false;
}

/**
 * TODO: Implement update_callback.
 */
bool update_callback(void *conn, const char *tid, const char **cols,
                     const char **vals) {
  UNUSED(conn);
  UNUSED(tid);
  UNUSED(cols);
  UNUSED(vals);
  return false;
}

#ifdef __cplusplus
}
#endif
