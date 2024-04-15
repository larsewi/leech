#include "files.h"

#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <memory.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "definitions.h"
#include "list.h"
#include "logger.h"
#include "string_lib.h"

/******************************************************************************/

bool LCH_FileSize(FILE *file, size_t *size) {
  if (fseek(file, 0, SEEK_END) != 0) {
    LCH_LOG_ERROR("Failed to seek to end of file: %s", strerror(errno));
    return false;
  }

  long pos = ftell(file);
  if (pos < 0) {
    LCH_LOG_ERROR("Failed to obtain the current file position indicator: %s",
                  strerror(errno));
    return false;
  }
  *size = (size_t)pos;

  if (fseek(file, 0, SEEK_SET) != 0) {
    LCH_LOG_ERROR("Failed to seek to start of file: %s", strerror(errno));
    return false;
  }

  return true;
}

/******************************************************************************/

bool LCH_FileExists(const char *const path) {
  struct stat sb;
  memset(&sb, 0, sizeof(struct stat));
  return stat(path, &sb) == 0;
}

/******************************************************************************/

bool LCH_FileIsRegular(const char *const path) {
  struct stat sb;
  memset(&sb, 0, sizeof(struct stat));
  return (stat(path, &sb) == 0) && ((sb.st_mode & S_IFMT) == S_IFREG);
}

/******************************************************************************/

bool LCH_FileIsDirectory(const char *const path) {
  struct stat sb;
  memset(&sb, 0, sizeof(struct stat));
  return (stat(path, &sb) == 0) && ((sb.st_mode & S_IFMT) == S_IFDIR);
}

/******************************************************************************/

bool LCH_FilePathJoin(char *path, const size_t path_max, const size_t n_items,
                      ...) {
  assert(path_max >= 1);

  va_list ap;
  va_start(ap, n_items);

  size_t used = 0;
  bool truncated = false;
  for (size_t i = 0; i < n_items; i++) {
    if (i > 0) {
      if (path_max - used < 2) {
        truncated = true;
        break;
      }
      path[used++] = LCH_PATH_SEP;
    }

    char *const sub = va_arg(ap, char *);
    const size_t sub_len = strlen(sub);
    for (size_t j = 0; j < sub_len; j++) {
      if (path_max - used < 2) {
        truncated = true;
        break;
      }
      path[used++] = sub[j];
    }
  }

  va_end(ap);
  path[used] = '\0';

  if (truncated) {
    LCH_LOG_ERROR("Failed to join paths: Truncation error.");
    return false;
  }
  return true;
}

/******************************************************************************/

bool LCH_FileDelete(const char *const filename) {
  if (unlink(filename) != 0) {
    LCH_LOG_ERROR("Failed to delete file '%s': %s", filename, strerror(errno));
    return false;
  }
  return true;
}

bool LCH_FileCreateParentDirectories(const char *const filename) {
  assert(filename != NULL);

  char fcopy[strlen(filename) + 1];
  strcpy(fcopy, filename);
  char *parent = dirname(fcopy);

  LCH_List *const dirs = LCH_ListCreate();
  struct stat sb;

  while (stat(parent, &sb) == -1) {
    char *const dir = LCH_StringDuplicate(parent);
    if (dir == NULL) {
      LCH_ListDestroy(dirs);
      return false;
    }

    if (!LCH_ListAppend(dirs, dir, free)) {
      free(dir);
      LCH_ListDestroy(dirs);
      return false;
    }

    parent = dirname(parent);
  }

  const size_t num_dirs = LCH_ListLength(dirs);
  for (size_t i = num_dirs; i > 0; i--) {
    char *const dir = (char *)LCH_ListGet(dirs, i - 1);
    if (mkdir(dir, (mode_t)0700) == -1) {
      LCH_LOG_ERROR("Failed to create parent directory '%s' for file '%s': %s",
                    dir, filename, strerror(errno));
      LCH_ListDestroy(dirs);
      return false;
    }
    LCH_LOG_VERBOSE("Created directory '%s' with mode %o", dir, (mode_t)0700);
  }
  LCH_ListDestroy(dirs);
  return true;
}
