#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "leech.h"
#include "definitions.h"

LCH_List *LCH_SplitString(const char *str, const char *del) {
  LCH_List *list = LCH_ListCreate();
  size_t to, from = 0, len = strlen(str);
  bool is_delim, was_delim = true;

  for (to = 0; to < len; to++) {
    is_delim = strchr(del, str[to]) != NULL;
    if (is_delim) {
      if (was_delim) {
        continue;
      }
      assert(to > from);
      char *s = strndup(str + from, to - from);
      if (s == NULL) {
        LCH_LOG_ERROR("Failed to allocate memory during string split: %s",
                      strerror(errno));
        LCH_ListDestroy(list);
        return NULL;
      }
      if (!LCH_ListAppend(list, (void *)s, free)) {
        free(s);
        LCH_ListDestroy(list);
        return NULL;
      }
    } else {
      if (was_delim) {
        from = to;
      }
    }
    was_delim = is_delim;
  }

  if (from < to && !is_delim) {
    char *s = strndup(str + from, to - from);
    if (s == NULL) {
      LCH_ListDestroy(list);
      return NULL;
    }
    if (!LCH_ListAppend(list, (void *)s, free)) {
      LCH_ListDestroy(list);
      return NULL;
    }
  }

  return list;
}

bool LCH_StringStartsWith(const char *const self, const char *const substr) {
  assert(self != NULL);
  assert(substr != NULL);

  size_t length = strlen(substr);
  for (size_t i = 0; i < length; i++) {
    if (self[i] != substr[i]) {
      return false;
    }
  }
  return true;
}

char *LCH_StringStrip(char *str, const char *charset) {
  assert(str != NULL);

  size_t start = 0, end = 0, cursor = 0;
  while (str[cursor] != '\0') {
    if (strchr(charset, str[cursor]) != NULL) {
      if (start == cursor) {
        ++start;
      }
    } else {
      end = cursor + 1;
    }
    ++cursor;
  }

  str = (char *)memmove((void *)str, (void *)(str + start), end - start);
  str[end - start] = '\0';
  return str;
}

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

bool LCH_FileExists(const char *const path) {
  struct stat sb = {0};
  return stat(path, &sb) == 0;
}

bool LCH_IsRegularFile(const char *const path) {
  struct stat sb = {0};
  return (stat(path, &sb) == 0) && ((sb.st_mode & S_IFMT) == S_IFREG);
}

bool LCH_IsDirectory(const char *const path) {
  struct stat sb = {0};
  return (stat(path, &sb) == 0) && ((sb.st_mode & S_IFMT) == S_IFDIR);
}

bool LCH_PathJoin(char *path, const size_t path_max, const size_t n_items, ...) {
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
      path[used++] = PATH_SEP;
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
