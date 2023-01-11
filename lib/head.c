#include "head.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"
#include "utils.h"

#define GENISIS_BLOCK_PARENT "0000000000000000000000000000000000000000"

char *LCH_HeadGet(const char *const work_dir) {
  assert(work_dir != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 2, work_dir, "HEAD")) {
    return NULL;
  }

  if (!LCH_IsRegularFile(path)) {
    return strdup(GENISIS_BLOCK_PARENT);
  }

  char *head = LCH_ReadTextFile(path, NULL);
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

  if (!LCH_WriteTextFile(path, block_id)) {
    return false;
  }

  return true;
}
