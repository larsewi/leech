#include <assert.h>
#include <errno.h>
#include <string.h>

#include "utils.h"

#define TEXTDATA(ch)                                                           \
  ((ch == '\t') || (ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) || \
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
    if (!LCH_BufferAppend(buffer, parser->cursor[0])) {
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

  char *const field = LCH_BufferToString(buffer);
  if (field == NULL) {
    LCH_LOG_ERROR(
        "Failed to create string from buffer for escaped field (Row %zu, Col "
        "%zu)",
        parser->row, parser->column);
    return NULL;
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
    if (!LCH_BufferAppend(buffer, parser->cursor[0])) {
      LCH_LOG_ERROR(
          "Failed to append character '%c' to buffer for non-escaped field "
          "(Row %zu, Col %zu)",
          parser->cursor[0], parser->row, parser->column);
      LCH_BufferDestroy(buffer);
      return NULL;
    }
    parser->cursor += 1;
  }

  char *const field = LCH_BufferToString(buffer);
  assert(field != NULL);
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
    free(field);
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

  char *field = ParseField(parser);
  if (field == NULL) {
    LCH_ListDestroy(record);
    return NULL;
  }

  if (!LCH_ListAppend(record, field, free)) {
    LCH_LOG_ERROR("Failed to append field '%s' to record (Row %zu, Col %zu)",
                  field, parser->row, parser->column);
    free(field);
    LCH_ListDestroy(record);
    return NULL;
  }

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

    if (!LCH_ListAppend(record, field, free)) {
      LCH_LOG_ERROR("Failed to append field '%s' to record (Row %zu, Col %zu)",
                    field, parser->row, parser->column);
      free(field);
      LCH_ListDestroy(record);
      return NULL;
    }
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

  LCH_List *record = ParseRecord(parser);
  if (record == NULL) {
    LCH_LOG_ERROR("Failed to parse record at (row %zu)", parser->row);
    LCH_ListDestroy(table);
    return NULL;
  }

  if (!LCH_ListAppend(table, record, LCH_ListDestroy)) {
    LCH_LOG_ERROR("Failed to append record to table (Row %zu)", parser->row);
    LCH_ListDestroy(record);
    LCH_ListDestroy(table);
    return NULL;
  }

  while (LCH_StringStartsWith(parser->cursor, "\r\n")) {
    parser->cursor += 2;

    if (parser->cursor[0] == '\0') {
      // This was just the optional trailing CRLF
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

    if (!LCH_ListAppend(table, record, LCH_ListDestroy)) {
      LCH_LOG_ERROR("Failed append record to table (Row %zu)", parser->row);
      LCH_ListDestroy(record);
      LCH_ListDestroy(table);
      return NULL;
    }
  }

  if (parser->cursor[0] != '\0') {
    LCH_LOG_ERROR("Expected EOF; found '%c' (Row %zu, Col %zu)",
                  parser->cursor[0], parser->row, parser->column);
    LCH_ListDestroy(table);
    return NULL;
  }
  return table;
}

char *LCH_CSVParseField(const char *const str) {
  assert(str != NULL);

  Parser parser = {
      .cursor = str,
      .row = 1,
      .column = 1,
  };

  char *const field = ParseField(&parser);
  if (field == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV field");
    return NULL;
  }
  return field;
}

LCH_List *LCH_CSVParseRecord(const char *const str) {
  assert(str != NULL);

  Parser parser = {
      .cursor = str,
      .row = 1,
      .column = 1,
  };

  LCH_List *const record = ParseRecord(&parser);
  if (record == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV record");
    return NULL;
  }
  return record;
}

LCH_List *LCH_CSVParseTable(const char *str) {
  assert(str != NULL);

  Parser parser = {
      .cursor = str,
      .row = 1,
      .column = 1,
  };

  LCH_List *const table = ParseTable(&parser);
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV");
    return NULL;
  }
  return table;
}

LCH_List *LCH_CSVParseFile(const char *const path) {
  char *csv = LCH_FileRead(path, NULL);
  if (csv == NULL) {
    LCH_LOG_ERROR("Failed to read CSV file '%s'.", path);
    return NULL;
  }

  LCH_List *table = LCH_CSVParseTable(csv);
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to parse CSV file '%s'", path, strerror(errno));
    free(csv);
    return NULL;
  }

  free(csv);
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
        if (!LCH_BufferPrintFormat(temp, "\"\"")) {
          LCH_BufferDestroy(temp);
          return false;
        }
        continue;
      }
    }

    if (!LCH_BufferAppend(temp, field[i])) {
      LCH_BufferDestroy(temp);
      return false;
    }
  }

  char *str = LCH_BufferToString(temp);
  assert(str != NULL);

  if (escape) {
    if (!LCH_BufferPrintFormat(buffer, "\"%s\"", str)) {
      free(str);
      return false;
    }
  } else {
    if (!LCH_BufferPrintFormat(buffer, "%s", str)) {
      free(str);
      return false;
    }
  }
  free(str);
  return true;
}

char *LCH_CSVComposeField(const char *const str) {
  LCH_Buffer *temp = LCH_BufferCreate();
  if (temp == NULL) {
    return NULL;
  }

  if (!ComposeField(temp, str)) {
    LCH_BufferDestroy(temp);
    return NULL;
  }

  char *field = LCH_BufferToString(temp);
  return field;
}

static bool ComposeRecord(LCH_Buffer *const buffer,
                          const LCH_List *const record) {
  assert(buffer != NULL);
  assert(record != NULL);

  const size_t length = LCH_ListLength(record);
  for (size_t i = 0; i < length; i++) {
    if (i > 0) {
      if (!LCH_BufferAppend(buffer, ',')) {
        return false;
      }
    }
    char *field = (char *)LCH_ListGet(record, i);
    if (!ComposeField(buffer, field)) {
      return false;
    }
  }

  return true;
}

bool LCH_CSVComposeRecord(LCH_Buffer **const buffer,
                          const LCH_List *const record) {
  assert(record != NULL);

  const bool create_buffer = *buffer == NULL;
  LCH_Buffer *const buf = (create_buffer) ? LCH_BufferCreate() : *buffer;
  if (buf == NULL) {
    return false;
  }
  const size_t offset = LCH_BufferLength(buf);

  if (!ComposeRecord(buf, record)) {
    LCH_LOG_ERROR("Failed to compose CSV record");
    if (create_buffer) {
      LCH_BufferDestroy(buf);
    } else {
      LCH_BufferChop(buf, offset);
    }
    return false;
  }

  *buffer = buf;
  return true;
}

bool LCH_CSVComposeTable(LCH_Buffer **const buffer,
                         const LCH_List *const table) {
  assert(table != NULL);
  assert(buffer != NULL);

  const bool create_buffer = *buffer == NULL;
  LCH_Buffer *const buf = (create_buffer) ? LCH_BufferCreate() : *buffer;
  if (buf == NULL) {
    return false;
  }
  const size_t offset = LCH_BufferLength(buf);

  const size_t length = LCH_ListLength(table);
  for (size_t i = 0; i < length; i++) {
    if (i > 0) {
      if (!LCH_BufferPrintFormat(buf, "\r\n")) {
        LCH_LOG_ERROR("Failed to compose CSV");
        if (create_buffer) {
          LCH_BufferDestroy(buf);
        } else {
          LCH_BufferChop(buf, offset);
        }
        return false;
      }
    }

    LCH_List *record = (LCH_List *)LCH_ListGet(table, i);
    if (!ComposeRecord(buf, record)) {
      LCH_LOG_ERROR("Failed to compose CSV");
      if (create_buffer) {
        LCH_BufferDestroy(buf);
      } else {
        LCH_BufferChop(buf, offset);
      }
      return false;
    }
  }

  *buffer = buf;
  return true;
}

bool LCH_CSVComposeFile(const LCH_List *table, const char *path) {
  assert(table != NULL);
  assert(path != NULL);

  LCH_Buffer *buffer = NULL;
  if (!LCH_CSVComposeTable(&buffer, table)) {
    return false;
  }

  if (!LCH_BufferWriteFile(buffer, path)) {
    return false;
  }

  LCH_BufferDestroy(buffer);
  return true;
}
