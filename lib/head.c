#include "head.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"
#include "utils.h"

char *LCH_HeadGet(const char *const work_dir) {
  assert(work_dir != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 2, work_dir, "HEAD")) {
    return NULL;
  }

  if (!LCH_IsRegularFile(path)) {
    return strdup(LCH_GENISIS_BLOCK_PARENT);
  }

  char *head = LCH_ReadFile(path, NULL);
  if (head == NULL) {
    return NULL;
  }

  return LCH_StringStrip(head, " \t\r\n");
}

bool LCH_HeadSet(const char *const workdir, const char *const block_id) {
  assert(workdir != NULL);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 2, workdir, "HEAD")) {
    return false;
  }

  if (!LCH_WriteFile(path, block_id)) {
    return false;
  }

  return true;
}
