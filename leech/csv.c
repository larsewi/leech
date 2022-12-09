#include "csv.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "debug_messenger.h"
#include "utils.h"

#define TEXTDATA(ch)                                           \
  ((ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) || \
   (ch >= 0x2D && ch <= 0x7E))

typedef struct Parser {
  const char *cursor;
  size_t row;
  size_t column;
} Parser;

// escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE
static char *ParseEscaped(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);

  // Remove leading double quote
  assert(parser->cursor[0] == '"');
  parser->cursor += 1;

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    LCH_LOG_ERROR(
        "Failed to create buffer for escaped field (Row %zu, Col %zu)",
        parser->row, parser->column);
    return NULL;
  }

  while (parser->cursor[0] != '"' ||
         LCH_StringStartsWith(parser->cursor, "\"\"")) {
    if (!LCH_BufferAppend(buffer, "%c", parser->cursor[0])) {
      LCH_LOG_ERROR(
          "Failed to append character '%c' to buffer for escaped field (Row "
          "%zu, Col %zu)",
          parser->cursor[0], parser->row, parser->column);
      LCH_BufferDestroy(buffer);
      return NULL;
    }
    parser->cursor += (parser->cursor[0] == '"') ? 2 : 1;
  }

  // Remove trailing double quote
  assert(parser->cursor[0] == '"');
  parser->cursor += 1;

  char *const field = LCH_BufferGet(buffer);
  LCH_BufferDestroy(buffer);
  if (field == NULL) {
    LCH_LOG_ERROR(
        "Failed to create string from buffer for escaped field (Row %zu, Col "
        "%zu)",
        parser->row, parser->column);
  }
  return field;
}

// non-escaped = *TEXTDATA
static char *ParseNonEscaped(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    LCH_LOG_ERROR(
        "Failed to create buffer for non-escaped field (Row %zu, Col %zu)",
        parser->row, parser->column);
    return NULL;
  }

  while (parser->cursor[0] != '\0' && parser->cursor[0] != ',' &&
         !LCH_StringStartsWith(parser->cursor, "\r\n")) {
    if (TEXTDATA(parser->cursor[0])) {
      if (!LCH_BufferAppend(buffer, "%c", parser->cursor[0])) {
        LCH_LOG_ERROR(
            "Failed to append character '%c' to buffer for non-escaped field "
            "(Row %zu, Col %zu)",
            parser->cursor[0], parser->row, parser->column);
        LCH_BufferDestroy(buffer);
      }
    } else {
      LCH_LOG_ERROR(
          "Expected 0x20-21 / 0x23-2B / 0x2D-7E; found '%c' (Row %zu, Col %zu)",
          parser->cursor[0], parser->row, parser->column);
      LCH_BufferDestroy(buffer);
    }
    parser->cursor += 1;
  }

  char *const field = LCH_BufferGet(buffer);
  LCH_BufferDestroy(buffer);
  if (field == NULL) {
    LCH_LOG_ERROR(
        "Failed to create string from buffer for non-escaped field (Row %zu, "
        "Col %zu)",
        parser->row, parser->column);
    return NULL;
  }

  return field;
}

// field = escaped / non-escaped
static char *ParseField(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);

  // Trim leading spaceses
  while (parser->cursor[0] == ' ') {
    parser->cursor += 1;
  }

  char *const field = (parser->cursor[0] == '"') ? ParseEscaped(parser)
                                                 : ParseNonEscaped(parser);
  if (field == NULL) {
    LCH_LOG_ERROR("Failed to parse field (Row %zu, Col %zu)", parser->row,
                  parser->column);
    return NULL;
  }

  // Trim trailing spaceses
  while (parser->cursor[0] == ' ') {
    parser->cursor += 1;
  }

  return field;
}

// record = field *(COMMA field)
static LCH_List *ParseRecord(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);

  LCH_List *const record = LCH_ListCreate();
  if (record == NULL) {
    LCH_LOG_ERROR("Failed to create record");
    return NULL;
  }
  LCH_LOG_DEBUG("Created empty record (Row %zu)", parser->row);

  char *field = ParseField(parser);
  if (field == NULL) {
    LCH_LOG_ERROR("Failed to parse field (Row %zu, Col %zu)", parser->row,
                  parser->column);
    LCH_ListDestroy(record);
    return NULL;
  }
  LCH_LOG_DEBUG("Parsed field '%s' (Row %zu, Col %zu)", field, parser->row,
                parser->column);

  if (!LCH_ListAppend(record, (void *)field, free)) {
    LCH_LOG_ERROR("Failed to append field '%s' to record (Row %zu, Col %zu)",
                  field, parser->row, parser->column);
    free(field);
    LCH_ListDestroy(record);
    return NULL;
  }
  LCH_LOG_DEBUG("Appended field '%s' to record (Row %zu, Col %zu)", field,
                parser->row, parser->column);

  while (parser->cursor[0] == ',') {
    parser->column += 1;
    parser->cursor += 1;

    field = ParseField(parser);
    if (field == NULL) {
      LCH_LOG_ERROR("Failed to parse field (Row %zu, Col %zu)", parser->row,
                    parser->column);
      LCH_ListDestroy(record);
      return NULL;
    }
    LCH_LOG_DEBUG("Parsed field '%s' (Row %zu, Col %zu)", field, parser->row,
                  parser->column);

    if (!LCH_ListAppend(record, (void *)field, free)) {
      LCH_LOG_ERROR("Failed to append field '%s' to record (Row %zu, Col %zu)",
                    field, parser->row, parser->column);
      free(field);
      LCH_ListDestroy(record);
      return NULL;
    }
    LCH_LOG_DEBUG("Appended field '%s' to record (Row %zu, Col %zu)", field,
                  parser->row, parser->column);
  }
  return record;
}

// table = record *(CRLF record) [CRLF]
static LCH_List *ParseTable(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);

  LCH_List *const table = LCH_ListCreate();
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to create table");
    return NULL;
  }
  LCH_LOG_DEBUG("Created table empty table");

  LCH_List *record = ParseRecord(parser);
  if (record == NULL) {
    LCH_LOG_ERROR("Failed to parse record at (row %zu)", parser->row);
    LCH_ListDestroy(table);
    return NULL;
  }
  LCH_LOG_DEBUG("Parsed record (Row %zu)", parser->row, parser->column);

  if (!LCH_ListAppend(table, (void *)record,
                      (void (*)(void *))LCH_ListDestroy)) {
    LCH_LOG_ERROR("Failed to append record to table (Row %zu)", parser->row);
    LCH_ListDestroy(record);
    LCH_ListDestroy(table);
    return NULL;
  }
  LCH_LOG_DEBUG("Appended record to table (Row %zu)", parser->row);

  while (LCH_StringStartsWith(parser->cursor, "\r\n")) {
    parser->cursor += 2;

    if (parser->cursor[0] == '\0') {
      // This was just the optional trailing CRLF
      LCH_LOG_DEBUG("Reached end of buffer");
      break;
    }

    parser->row += 1;
    parser->column = 1;

    record = ParseRecord(parser);
    if (record == NULL) {
      LCH_LOG_ERROR("Failed to parse record (row %zu)", parser->row);
      LCH_ListDestroy(table);
      return NULL;
    }
    LCH_LOG_DEBUG("Parsed record (Row %zu)", parser->row);

    if (!LCH_ListAppend(table, (void *)record,
                        (void (*)(void *))LCH_ListDestroy)) {
      LCH_LOG_ERROR("Failed append record to table (Row %zu)", parser->row);
      LCH_ListDestroy(record);
      LCH_ListDestroy(table);
      return NULL;
    }
    LCH_LOG_DEBUG("Appended record to table (Row %zu)", parser->row);
  }

  if (parser->cursor[0] != '\0') {
    LCH_LOG_ERROR("Expected EOF; found '%c' (Row %zu, Col %zu)",
                  parser->cursor[0], parser->row, parser->column);
    LCH_ListDestroy(table);
    return NULL;
  }
  return table;
}

LCH_List *LCH_ParseCSV(const char *str) {
  assert(str != NULL);

  Parser parser = {
      .cursor = str,
      .row = 1,
      .column = 1,
  };

  LCH_List *const table = ParseTable(&parser);
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV");
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
  } else {
    if (!LCH_BufferAppend(buffer, str)) {
      free(str);
      return false;
    }
  }
  LCH_LOG_DEBUG("Composed field '%s'", str);
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
    }
    char *field = (char *)LCH_ListGet(record, i);
    if (!ComposeField(buffer, field)) {
      return false;
    }
    LCH_LOG_DEBUG("Appended field '%s' to record", field);
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
    LCH_LOG_DEBUG("Appended record to table");
  }
  return buffer;
}
