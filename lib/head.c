#include "head.h"

#include <errno.h>
#include <string.h>
#include <limits.h>

#include "leech.h"
#include "utils.h"
#include "definitions.h"

char *LCH_HeadGet(const char *const work_dir) {
  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 2, work_dir, "HEAD")) {
    return NULL;
  }

  if (!LCH_IsRegularFile(path)) {
    return strdup("0000000000000000000000000000000000000000");
  }

  char *head = LCH_ReadTextFile(path, NULL);
  if (head == NULL) {
    return NULL;
  }

  return LCH_StringStrip(head, " \t\r\n");
}

bool LCH_HeadSet(const char *const workdir, const char *const block_id) {
  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 2, workdir, "HEAD")) {
    return NULL;
  }

  UNUSED(block_id);

  return false;
}
