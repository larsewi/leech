#include "csv.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "utils.h"
#include "debug_messenger.h"

#define TEXTDATA(ch)                                           \
  ((ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) || \
   (ch >= 0x2D && ch <= 0x7E))

typedef enum State {
  START_TABLE,
  START_RECORD,
  START_FIELD,
  PROCESS_FIELD,
  END_FIELD,
  END_RECORD,
  END_TABLE,
  ERROR,
} State;

LCH_List *LCH_ParseCSV(const char *str) {
  assert(str != NULL);

  LCH_List *table = NULL;
  LCH_List *record = NULL;
  LCH_Buffer *field = NULL;

  State state = START_TABLE;
  bool escaped = false;
  const char *cursor = str;

  while (*cursor != '\0') {
    switch (state) {
      case START_TABLE: {
        table = LCH_ListCreate();
        state = (table != NULL) ? START_RECORD : ERROR;
      } break;

      case START_RECORD: {
        record = LCH_ListCreate();
        state = (record != NULL) ? START_FIELD : ERROR;
      } break;

      case START_FIELD: {
        field = LCH_BufferCreate();
        if (*cursor == ' ') {
          ++cursor;
          break;
        }
        state = (field != NULL) ? PROCESS_FIELD : ERROR;
      } break;

      case PROCESS_FIELD: {
        if (!escaped && *cursor == ',') {
          state = END_FIELD;
          break;
        }

        if (!TEXTDATA(*cursor)) {
          escaped = true;
        }
      };

      case END_FIELD: {
      };

      case END_RECORD: {
      };

      case END_TABLE: {
      };

      case ERROR: {
        LCH_LOG_ERROR("Failed to parse CSV");
        return NULL;
      };

      default:
        assert(false);
        break;
    }
  }
  return table;
}

static bool ComposeField(LCH_Buffer *const buffer, const char *const field) {
  assert(buffer != NULL);
  assert(field != NULL);

  LCH_Buffer *temp = LCH_BufferCreate();
  if (temp == NULL) {
    return false;
  }

  const size_t length = strlen(field);

  /* Fields starting with or ending with a space should be escaped */
  bool escape = length > 0 && (field[0] == ' ' || field[length - 1] == ' ');

  for (size_t i = 0; i < length; i++) {
    if (!TEXTDATA(field[i])) {
      escape = true;

      if (field[i] == '"') {
        if (!LCH_BufferAppend(temp, "\"\"")) {
          LCH_BufferDestroy(temp);
          return false;
        }
        LCH_LOG_DEBUG("Escaped double quote");
        continue;
      }
    }

    if (!LCH_BufferAppend(temp, "%c", field[i])) {
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
    LCH_LOG_DEBUG("Composed escaped field: \"%s\"", str);
  } else {
    if (!LCH_BufferAppend(buffer, str)) {
      free(str);
      return false;
    }
    LCH_LOG_DEBUG("Composed non-escaped field: %s", str);
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
      LCH_LOG_DEBUG("Added field separator");
    }
    char *field = (char *)LCH_ListGet(record, i);
    if (!ComposeField(buffer, field)) {
      return false;
    }
    LCH_LOG_DEBUG("Added field");
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
      LCH_LOG_DEBUG("Added record separator");
    }

    LCH_List *record = (LCH_List *)LCH_ListGet(table, i);
    if (!ComposeRecord(buffer, record)) {
      LCH_LOG_ERROR("Failed to compose CSV");
      LCH_BufferDestroy(buffer);
      return NULL;
    }
    LCH_LOG_DEBUG("Added record");
  }

  return buffer;
}
