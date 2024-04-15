#include "head.h"

#include <assert.h>
#include <limits.h>

#include "buffer.h"
#include "definitions.h"
#include "files.h"
#include "logger.h"
#include "string_lib.h"

char *LCH_HeadGet(const char *const name, const char *const work_dir) {
  assert(work_dir != NULL);

  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, PATH_MAX, 2, work_dir, name)) {
    return NULL;
  }

  if (LCH_FileExists(path)) {
    LCH_Buffer *const buffer = LCH_BufferCreate();
    if (buffer == NULL) {
      return NULL;
    }

    if (!LCH_BufferReadFile(buffer, path)) {
      LCH_BufferDestroy(buffer);
      return NULL;
    }

    char *const block_id = LCH_BufferToString(buffer);
    if (block_id == NULL) {
      return NULL;
    }

    LCH_StringStrip(block_id, " \t\r\n");
    LCH_LOG_DEBUG("Loaded head %.7s", block_id);

    return block_id;
  }

  LCH_LOG_DEBUG("Head does not exist, returning genisis block identifier");
  char *const block_id = LCH_StringDuplicate(LCH_GENISIS_BLOCK_ID);

  return block_id;
}

bool LCH_HeadSet(const char *const name, const char *const work_dir,
                 const char *const block_id) {
  assert(work_dir != NULL);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, PATH_MAX, 2, work_dir, name)) {
    return false;
  }

  LCH_Buffer *const buffer = LCH_BufferFromString(block_id);
  if (buffer == NULL) {
    return false;
  }

  if (!LCH_BufferWriteFile(buffer, path)) {
    LCH_BufferDestroy(buffer);
    return false;
  }

  LCH_BufferDestroy(buffer);
  LCH_LOG_DEBUG("Moved head to %s in '%s'", block_id, path);

  return true;
}
