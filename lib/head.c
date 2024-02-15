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
  if (!LCH_PathJoin(path, PATH_MAX, 2, work_dir, "HEAD")) {
    return NULL;
  }

  if (LCH_FileExists(path)) {
    char *const block_id = LCH_FileRead(path, NULL);
    if (block_id == NULL) {
      return NULL;
    }
    LCH_StringStrip(block_id, " \t\r\n");
    LCH_LOG_DEBUG("Loaded HEAD %.7s", block_id);
    return block_id;
  }

  LCH_LOG_DEBUG("HEAD does not exist, returning genisis block identifier");
  char *const block_id = LCH_StringDuplicate(LCH_GENISIS_BLOCK_ID);

  return block_id;
}

bool LCH_HeadSet(const char *const work_dir, const char *const block_id) {
  assert(work_dir != NULL);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, PATH_MAX, 2, work_dir, "HEAD")) {
    return false;
  }

  if (!LCH_FileWrite(path, block_id)) {
    return false;
  }

  LCH_LOG_DEBUG("Moved HEAD to %s in '%s'", block_id, path);
  return true;
}
