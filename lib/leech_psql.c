#include <stdbool.h>
#include <stdlib.h>

#include "definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TODO: Implement load_callback.
 */
char ***load_callback(void *locator) {
  LCH_UNUSED(locator);
  return NULL;
}

/**
 * TODO: Implement begin_tx_callback.
 */
void *begin_tx_callback(void *locator) {
  LCH_UNUSED(locator);
  return NULL;
}

/**
 * TODO: Implement end_tx_callback.
 */
bool end_tx_callback(void *conn, int err) {
  LCH_UNUSED(conn);
  LCH_UNUSED(err);
  return false;
}

/**
 * TODO: Implement insert_callback.
 */
bool insert_callback(void *conn, const char *tid, const char *const *cols,
                     const char *const *vals) {
  LCH_UNUSED(conn);
  LCH_UNUSED(tid);
  LCH_UNUSED(cols);
  LCH_UNUSED(vals);
  return false;
}

/**
 * TODO: Implement delete_callback.
 */
bool delete_callback(void *conn, const char *tid, const char *const *cols,
                     const char *const *vals) {
  LCH_UNUSED(conn);
  LCH_UNUSED(tid);
  LCH_UNUSED(cols);
  LCH_UNUSED(vals);
  return false;
}

/**
 * TODO: Implement update_callback.
 */
bool update_callback(void *conn, const char *tid, const char **cols,
                     const char **vals) {
  LCH_UNUSED(conn);
  LCH_UNUSED(tid);
  LCH_UNUSED(cols);
  LCH_UNUSED(vals);
  return false;
}

#ifdef __cplusplus
}
#endif
