/**
 * This file contains bogus callbacks, to check that they can be called from
 * check_table.c.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

char ***load_callback(void *locator) {
  char ***table = (char ***)malloc(2 * sizeof(char **));
  table[0] = (char **)malloc(2 * sizeof(char *));
  table[0][0] = strdup((const char *)locator);
  table[0][1] = NULL;
  table[1] = NULL;
  return table;
}

void *begin_tx_callback(void *locator) { return locator; }

bool end_tx_callback(void *conn, int err) {
  const char *str = (char *)conn;
  if (strcmp(str, "Hello CFEngine") == 0 && err == 3) {
    return true;
  }
  return false;
}

bool insert_callback(void *conn, const char *tid, const char *const *cols,
                     const char *const *vals) {
  const char *str = (char *)conn;
  if (strcmp(str, "insert") != 0) {
    return false;
  }
  if (strcmp(tid, "foo") != 0) {
    return false;
  }
  if (strcmp(cols[0], "bar") != 0) {
    return false;
  }
  if (strcmp(vals[0], "baz") != 0) {
    return false;
  }
  return true;
}

bool delete_callback(void *conn, const char *tid, const char *const *cols,
                     const char *const *vals) {
  const char *str = (char *)conn;
  if (strcmp(str, "delete") != 0) {
    return false;
  }
  if (strcmp(tid, "foo") != 0) {
    return false;
  }
  if (strcmp(cols[0], "bar") != 0) {
    return false;
  }
  if (strcmp(vals[0], "baz") != 0) {
    return false;
  }
  return true;
}

bool update_callback(void *conn, const char *tid, const char **cols,
                     const char **vals) {
  const char *str = (char *)conn;
  if (strcmp(str, "update") != 0) {
    return false;
  }
  if (strcmp(tid, "foo") != 0) {
    return false;
  }
  if (strcmp(cols[0], "bar") != 0) {
    return false;
  }
  if (strcmp(vals[0], "baz") != 0) {
    return false;
  }
  return true;
}

#ifdef __cplusplus
}
#endif
