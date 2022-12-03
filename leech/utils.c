#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "debug_messenger.h"
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
        LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
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

char *LCH_StringStrip(char *self) {
  assert(self != NULL);

  size_t start = 0, end = 0, cursor = 0;
  while (self[cursor] != '\0') {
    if (self[cursor] == ' ') {
      if (start == cursor) {
        ++start;
      }
    } else {
      end = cursor + 1;
    }
    ++cursor;
  }
  LCH_LOG_DEBUG("strlen: %zu", strlen(self));
  LCH_LOG_DEBUG("Start: %zu, end: %zu", start, end);
  LCH_LOG_DEBUG("Dest: %p, src: %p, n: %zu", self, self + start, end - start);

  self = (char *)memmove((void *)self, (void *)(self + start), end - start);
  self[end - start] = '\0';
  return self;
}

// bool LCH_FileWriteCSVField(FILE *const file, const char *const field) {
//   assert(file != NULL);
//   assert(field != NULL);

//   bool escaped = false;
//   for (size_t i = 0; i < strlen(field); i++) {
//     if (!TEXT_DATA(field[i])) {
//       escaped = true;
//       break;
//     }
//   }

//   if (escaped) {
//     if (fputc('"', file) == EOF) {
//       return false;
//     }
//   }

//   for (size_t i = 0; i < strlen(field); i++) {
//     if (field[i] == '"') {
//       if (fputc('"', file) == EOF) {
//         return false;
//       }
//     }

//     if (fputc(field[i], file) == EOF) {
//       return false;
//     }
//   }

//   if (escaped) {
//     if (fputc('"', file) == EOF) {
//       return false;
//     }
//   }

//   return true;
// }

// bool LCH_FileWriteCSVRecord(FILE *const file, const LCH_List *const record) {
//   assert(file != NULL);
//   assert(record != NULL);

//   const size_t length = LCH_ListLength(record);
//   for (int i = 0; i < length; i++) {
//     if (i > 0) {
//       if (fputc(',', file) == EOF) {
//         return false;
//       }
//     }

//     const char *const field = (char *)LCH_ListGet(record, i);
//     if (!LCH_FileWriteCSVField(file, field)) {
//       return false;
//     }
//   }

//   return true;
// }

// bool LCH_FileWriteCSVTable(FILE *const file, const LCH_List *const table) {
//   assert(file != NULL);
//   assert(table != NULL);

//   const size_t length = LCH_ListLength(table);
//   for (size_t i = 0; i < length; i++) {
//     if (i > 0) {
//       char crlf[] = "\r\n";
//       if (fputs(crlf, file) == EOF) {
//         return false;
//       }
//     }

//     const LCH_List *const record = (LCH_List *)LCH_ListGet(table, i);
//     if (!LCH_FileWriteCSVRecord(file, record)) {
//       return false;
//     }
//   }

//   return true;
// }
