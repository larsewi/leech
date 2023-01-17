#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "definitions.h"
#include "leech.h"

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

static bool SplitStringSubstringMaybeAddElement(LCH_List *const lst,
                                                const char *const str,
                                                const size_t from,
                                                const size_t to) {
  if (from < to) {
    char *const item = strndup(str + from, to - from);
    if (item == NULL) {
      LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
      return false;
    }

    if (!LCH_ListAppend(lst, item, free)) {
      free(item);
      return false;
    }
  }

  return true;
}

LCH_List *LCH_SplitStringSubstring(const char *const str,
                                   const char *const substr) {
  assert(str != NULL);
  assert(substr != NULL);

  LCH_List *const lst = LCH_ListCreate();
  if (lst == NULL) {
    return NULL;
  }

  size_t from = 0, to;
  for (to = 0; str[to] != '\0'; to++) {
    size_t i;
    bool is_match = true;
    for (i = 0; substr[i] != '\0'; i++) {
      if (str[to + i] != substr[i]) {
        is_match = false;
        break;
      }
    }

    if (is_match) {
      if (!SplitStringSubstringMaybeAddElement(lst, str, from, to)) {
        LCH_ListDestroy(lst);
        return NULL;
      }
      to += i;
      from = to;
    }
  }

  if (!SplitStringSubstringMaybeAddElement(lst, str, from, to)) {
    LCH_ListDestroy(lst);
    return NULL;
  }

  return lst;
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

bool LCH_PathJoin(char *path, const size_t path_max, const size_t n_items,
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

char *LCH_ReadTextFile(const char *const path, size_t *const length) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for reading: %s", path,
                  strerror(errno));
    return NULL;
  }

  char *buffer = NULL;
  size_t buffer_size = LCH_BUFFER_SIZE;
  size_t total_read = 0, bytes_read = 0;

  do {
    char *ptr = realloc(buffer, buffer_size);
    if (ptr == NULL) {
      LCH_LOG_ERROR(
          "Failed to reallocate (%zu bytes) memory for read buffer: %s",
          buffer_size, strerror(errno));
      free(buffer);
      fclose(file);
      return NULL;
    }

    buffer = ptr;
    bytes_read =
        fread(buffer + total_read, 1, buffer_size - total_read - 1, file);
    total_read += bytes_read;
    buffer_size *= LCH_BUFFER_SIZE;
  } while (bytes_read != 0);

  if (ferror(file)) {
    LCH_LOG_ERROR("Failed to read file '%s'.", path);
    free(buffer);
    fclose(file);
    return NULL;
  }
  fclose(file);

  if (length != NULL) {
    *length = total_read;
  }
  buffer[total_read] = '\0';

  return buffer;
}

bool LCH_WriteTextFile(const char *const path, const char *const str) {
  FILE *file = fopen(path, "w");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for writing: %s", path,
                  strerror(errno));
    return false;
  }

  if (fputs(str, file) == EOF) {
    LCH_LOG_ERROR("Failed to write to file '%s'.", path);
    fclose(file);
    return false;
  }

  fclose(file);
  return true;
}
