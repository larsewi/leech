#include "csv.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "debug_messenger.h"

#define TEXTDATA(ch) ((ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) || (ch >= 0x2D && ch <= 0x7E))


LCH_List *LCH_ParseCSV(const char *const str) { assert(str != NULL); }

static bool ComposeField(LCH_Buffer *const buffer,
                         const char *const field) {
  assert(buffer != NULL);
  assert(field != NULL);

  LCH_Buffer *temp = LCH_BufferCreate();
  if (temp == NULL) {
    return false;
  }

  bool escape = false;
  for (char ch = *field; ch != '\0'; ch++) {
    if (!TEXTDATA(ch)) {
      escape = true;

      if (ch == '"') {
        if (!LCH_BufferAppend(temp, "\"\"")) {
          LCH_BufferDestroy(temp);
          return false;
        }
        continue;
      }
    }

    if (!LCH_BufferAppend(temp, "%c", ch)) {
      LCH_BufferDestroy(temp);
      return false;
    }
  }

  char *str = LCH_BufferGet(temp);
  LCH_BufferDestroy(temp);
  if (str == NULL) {
    return false;
  }

  if (escape) {
    if (!LCH_BufferAppend(buffer, "\"%s\"", str)) {
      free(str);
      return false;
    }
  } else {
    if (!LCH_BufferAppend(buffer, str)) {
      free(str);
      return false;
    }
  }
  free(str);
  return true;
}

static bool ComposeRecord(LCH_Buffer *const buffer,
                          const LCH_List *const record) {
  assert(buffer != NULL);
  assert(record != NULL);

  const size_t length = LCH_ListLength(record);
  for (size_t i = 0; i < length; i++) {
    if (i > 0) {
      if (!LCH_BufferAppend(buffer, ",")) {
        return false;
      }

      LCH_List *field = (LCH_List *)LCH_ListGet(record, i);
      if (!ComposeField(buffer, field)) {
        return false;
      }
    }
  }

  return true;
}

LCH_Buffer *LCH_ComposeCSV(const LCH_List *const table) {
  assert(table != NULL);

  LCH_Buffer *buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to compose CSV");
    return NULL;
  }

  const size_t length = LCH_ListLength(table);
  for (size_t i = 0; i < length; i++) {
    if (i > 0) {
      if (!LCH_BufferAppend(buffer, "\r\n")) {
        LCH_LOG_ERROR("Failed to compose CSV");
        LCH_BufferDestroy(buffer);
        return NULL;
      }
    }

    LCH_List *record = (LCH_List *)LCH_ListGet(table, i);
    if (!ComposeRecord(buffer, record)) {
      LCH_LOG_ERROR("Failed to compose CSV");
      LCH_BufferDestroy(buffer);
      return NULL;
    }
  }

  return buffer;
}
