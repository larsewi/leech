#include "string_lib.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "definitions.h"
#include "logger.h"

/******************************************************************************/

bool LCH_StringEqual(const char *const str1, const char *const str2) {
  assert(str1 != NULL);
  assert(str2 != NULL);
  return strcmp(str1, str2) == 0;
}

/******************************************************************************/

LCH_List *LCH_StringSplit(const char *str, const char *del) {
  assert(str != NULL);
  assert(del != NULL);

  LCH_List *const list = LCH_ListCreate();
  if (list == NULL) {
    return NULL;
  }

  const char *start = str;
  const char *end = strpbrk(str, del);

  while (end != NULL) {
    char *tmp = strndup(start, end - start);
    if (tmp == NULL) {
      LCH_LOG_ERROR("strndup(3): Failed to allocate memory: %s",
                    strerror(errno));
      return NULL;
    }

    if (!LCH_ListAppend(list, tmp, free)) {
      free(tmp);
      LCH_ListDestroy(list);
      return NULL;
    }

    start = end + 1;
    end = strpbrk(start, del);
  }

  char *tmp = LCH_StringDuplicate(start);
  if (tmp == NULL) {
    LCH_ListDestroy(list);
    return NULL;
  }

  if (!LCH_ListAppend(list, tmp, free)) {
    free(tmp);
    LCH_ListDestroy(list);
    return NULL;
  }

  return list;
}

/******************************************************************************/

char *LCH_StringJoin(const LCH_List *const list, const char *const del) {
  LCH_Buffer *const buffer = LCH_BufferCreate();

  const size_t len = LCH_ListLength(list);
  for (size_t i = 0; i < len; i++) {
    if (i > 0) {
      if (!LCH_BufferPrintFormat(buffer, "%s", del)) {
        LCH_BufferDestroy(buffer);
        return NULL;
      }
    }

    const char *const str = (const char *)LCH_ListGet(list, i);
    if (!LCH_BufferPrintFormat(buffer, "%s", str)) {
      LCH_BufferDestroy(buffer);
      return NULL;
    }
  }

  char *const result = LCH_BufferToString(buffer);
  return result;
}

/******************************************************************************/

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

/******************************************************************************/

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

  str = (char *)memmove(str, (str + start), end - start);
  str[end - start] = '\0';
  return str;
}

/******************************************************************************/

bool LCH_StringParseNumber(const char *const str, long *const number) {
  assert(str != NULL);
  assert(number != NULL);

  char *endptr;
  errno = 0;  // To distinguish success/failure after call
  const long value = strtol(str, &endptr, 10);

  if (errno != 0) {
    LCH_LOG_ERROR("Failed to parse number '%s': %s", str, strerror(errno));
    return false;
  }

  if (endptr == str) {
    LCH_LOG_ERROR("Failed to parse number '%s': No digits were found", str);
    return false;
  }

  if (*endptr != '\0') {
    LCH_LOG_WARNING(
        "Found trailing characters '%s' after parsing number '%ld' from string "
        "'%s'",
        endptr, value, str);
  }

  *number = value;
  return true;
}

/******************************************************************************/

bool LCH_StringParseVersion(const char *const str, size_t *const v_major,
                            size_t *const v_minor, size_t *const v_patch) {
  assert(str != NULL);
  assert(v_major != NULL);
  assert(v_minor != NULL);
  assert(v_patch != NULL);

  LCH_List *const list = LCH_StringSplit(str, ".");
  const size_t length = LCH_ListLength(list);

  static const char *const error_messages[] = {
      "Missing major version number",
      "Missing minor version number",
      "Missing patch version number",
      "Too many version numbers",
  };
  if (length < 3 || length > 3) {
    LCH_LOG_ERROR("Failed to parse version '%s': %s",
                  error_messages[LCH_MIN(length, 3)]);
    LCH_ListDestroy(list);
    return false;
  }

  long val;
  const char *sub = (char *)LCH_ListGet(list, 0);
  if (!LCH_StringParseNumber(sub, &val)) {
    LCH_ListDestroy(list);
    return false;
  }
  if (val < 0) {
    LCH_LOG_ERROR("Failed to parse version '%s': Bad major version number %ld",
                  str, val);
    LCH_ListDestroy(list);
    return false;
  }
  *v_major = (size_t)val;

  sub = (char *)LCH_ListGet(list, 1);
  if (!LCH_StringParseNumber(sub, &val)) {
    LCH_ListDestroy(list);
    return false;
  }
  if (val < 0) {
    LCH_LOG_ERROR("Failed to parse version '%s': Bad major version number %ld",
                  str, val);
    LCH_ListDestroy(list);
    return false;
  }
  *v_minor = (size_t)val;

  sub = (char *)LCH_ListGet(list, 2);
  if (!LCH_StringParseNumber(sub, &val)) {
    LCH_ListDestroy(list);
    return false;
  }
  if (val < 0) {
    LCH_LOG_ERROR("Failed to parse version '%s': Bad major version number %ld",
                  str, val);
    LCH_ListDestroy(list);
    return false;
  }

  LCH_ListDestroy(list);
  *v_patch = (size_t)val;
  return true;
}

/******************************************************************************/

char *LCH_StringFormat(const char *const format, ...) {
  assert(format != NULL);

  va_list ap;
  va_start(ap, format);
  const int length = vsnprintf(NULL, 0, format, ap);
  assert(length >= 0);
  va_end(ap);

  char *const str = (char *)malloc((size_t)length + 1);
  if (str == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  va_start(ap, format);
  LCH_NDEBUG_UNUSED const int ret =
      vsnprintf(str, (size_t)length + 1, format, ap);
  va_end(ap);
  assert(ret == length);

  return str;
}

/******************************************************************************/

char *LCH_StringTruncate(const char *const str, const size_t len,
                         const size_t max) {
  assert(max >= 3);  // We need at least Bytes for ...

  LCH_Buffer *const buffer = LCH_BufferCreate();
  for (size_t i = 0; i < max; i++) {
    if ((i < len) && (str[i] == '\0')) {
      return LCH_BufferToString(buffer);
    }
    if (!LCH_BufferAppend(buffer, str[i])) {
      LCH_BufferDestroy(buffer);
      return NULL;
    }
  }

  LCH_BufferChop(buffer, max - 3);
  if (!LCH_BufferPrintFormat(buffer, "...")) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  return LCH_BufferToString(buffer);
}

/******************************************************************************/

char *LCH_StringDuplicate(const char *const str) {
  if (str == NULL) {
    return NULL;
  }

  char *const dup = strdup(str);
  if (dup == NULL) {
    LCH_LOG_ERROR("strdup(3): Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }
  return dup;
}
