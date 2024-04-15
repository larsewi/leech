#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "files.h"
#include "logger.h"
#include "utils.h"

/**
 * Character values allowed in non-escaped fields
 */
#define TEXTDATA(ch)                                                           \
  ((ch == '\t') || (ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) || \
   (ch >= 0x2D && ch <= 0x7E))

typedef struct LCH_CSVParser {
  const char *cursor;     // Current position in buffer
  const char *const end;  // End of buffer
  size_t row;             // Current row number (used in error messages)
  size_t column;          // Current column number (used in error messages)
} LCH_CSVParser;

/**
 * escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE
 */
static bool ParseEscaped(LCH_CSVParser *const parser, LCH_Buffer *const field) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);
  assert(field != NULL);

  // Remove leading double quote
  assert(parser->cursor[0] == '"');
  parser->cursor += 1;

  while (parser->cursor < parser->end) {
    if (((parser->cursor + 1) < parser->end) &&
        (parser->cursor[0] == '"' && parser->cursor[1] == '"')) {
      // Found escaped quote in field
      if (!LCH_BufferAppend(field, '"')) {
        return false;
      }
      parser->cursor += 2;
    } else if (parser->cursor[0] == '"') {
      // Reached end of field
      parser->cursor += 1;

      // Trim trailing spaces
      while (((parser->cursor) < parser->end) && (parser->cursor[0] == ' ')) {
        parser->cursor += 1;
      }

      if (parser->cursor >= parser->end) {
        // Reached End-of-Buffer
        return true;
      }

      if ((parser->cursor < parser->end) && (parser->cursor[0] == ',')) {
        // Reached End-of-Field
        return true;
      }

      if ((parser->cursor + 1 < parser->end) && (parser->cursor[0] == '\r') &&
          (parser->cursor[1] == '\n')) {
        // Reached End-of-Record
        return true;
      }

      LCH_LOG_ERROR(
          "Failed to parse CSV: Expected End-of-Buffer, COMMA or CRLF, but "
          "found token %#02x (Row %zu, Col %zu)",
          parser->cursor[0], parser->row, parser->column);
      return false;
    } else {
      // Found byte
      if (!LCH_BufferAppend(field, parser->cursor[0])) {
        return false;
      }
      parser->cursor += 1;
    }
  }
  LCH_LOG_ERROR(
      "Failed to parse CSV: Expected DQUOTE, but reached End-of-Buffer (Row "
      "%zu, Col %zu)",
      parser->row, parser->column);
  return false;
}

/**
 * non-escaped = *TEXTDATA
 */
static bool ParseNonEscaped(LCH_CSVParser *const parser,
                            LCH_Buffer *const field) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);
  assert(field != NULL);

  while (parser->cursor < parser->end) {
    if (((parser->cursor + 1) < parser->end) && parser->cursor[0] == '\r' &&
        parser->cursor[1] == '\n') {
      // Reached end of record
      break;
    } else if (parser->cursor[0] == ',') {
      // Reached end of field
      break;
    } else if (TEXTDATA(parser->cursor[0])) {
      // Found text data
      if (!LCH_BufferAppend(field, parser->cursor[0])) {
        return false;
      }
      parser->cursor += 1;
    } else {
      LCH_LOG_ERROR(
          "Failed to parse CSV: Expected End-of-Buffer, TEXTDATA, COMMA or "
          "CRLF, but found token %#02x (Row %zu, Col %zu)",
          parser->cursor[0], parser->row, parser->column);
      return false;
    }
  }
  LCH_BufferTrim(field, ' ');  // Remove trailing spaces
  return true;
}

/**
 * field = escaped / non-escaped
 */
static LCH_Buffer *ParseField(LCH_CSVParser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);

  // Trim leading spaces
  while ((parser->cursor < parser->end) && (parser->cursor[0] == ' ')) {
    parser->cursor += 1;
  }

  LCH_Buffer *const field = LCH_BufferCreate();
  if (field == NULL) {
    return NULL;
  }

  if (parser->cursor < parser->end) {
    if (parser->cursor[0] == '"') {
      if (!ParseEscaped(parser, field)) {
        LCH_BufferDestroy(field);
        return NULL;
      }
    } else {
      if (!ParseNonEscaped(parser, field)) {
        LCH_BufferDestroy(field);
        return NULL;
      }
    }
  }

  return field;
}

/**
 * record = field *(COMMA field)
 */
static LCH_List *ParseRecord(LCH_CSVParser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);

  LCH_List *const record = LCH_ListCreate();
  if (record == NULL) {
    return NULL;
  }

  LCH_Buffer *field = ParseField(parser);
  if (field == NULL) {
    LCH_ListDestroy(record);
    return NULL;
  }

  if (!LCH_ListAppend(record, field, LCH_BufferDestroy)) {
    LCH_BufferDestroy(field);
    LCH_ListDestroy(record);
    return NULL;
  }

  while ((parser->cursor < parser->end) && (parser->cursor[0] == ',')) {
    parser->column += 1;
    parser->cursor += 1;

    field = ParseField(parser);
    if (field == NULL) {
      LCH_ListDestroy(record);
      return NULL;
    }

    if (!LCH_ListAppend(record, field, LCH_BufferDestroy)) {
      LCH_BufferDestroy(field);
      LCH_ListDestroy(record);
      return NULL;
    }
  }
  return record;
}

/**
 * table = record *(CRLF record) [CRLF]
 */
static LCH_List *ParseTable(LCH_CSVParser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);

  LCH_List *const table = LCH_ListCreate();
  if (table == NULL) {
    return NULL;
  }

  LCH_List *record = ParseRecord(parser);
  if (record == NULL) {
    LCH_ListDestroy(table);
    return NULL;
  }

  if (!LCH_ListAppend(table, record, LCH_ListDestroy)) {
    LCH_ListDestroy(record);
    LCH_ListDestroy(table);
    return NULL;
  }

  while (parser->cursor < parser->end) {
    assert(parser->cursor + 1 < parser->end);
    assert(parser->cursor[0] == '\r');
    assert(parser->cursor[1] == '\n');
    parser->cursor += 2;

    if (parser->cursor >= parser->end) {
      // This was just the optional trailing CRLF
      break;
    }

    parser->row += 1;
    parser->column = 1;

    record = ParseRecord(parser);
    if (record == NULL) {
      LCH_ListDestroy(table);
      return NULL;
    }

    if (!LCH_ListAppend(table, record, LCH_ListDestroy)) {
      LCH_ListDestroy(record);
      LCH_ListDestroy(table);
      return NULL;
    }
  }

  assert(parser->cursor == parser->end);
  return table;
}

LCH_Buffer *LCH_CSVParseField(const char *const csv, const size_t size) {
  assert(csv != NULL);

  LCH_CSVParser parser = {
      .cursor = csv,
      .end = csv + size,
      .row = 1,
      .column = 1,
  };

  LCH_Buffer *const field = ParseField(&parser);
  if (field == NULL) {
    return NULL;
  }
  return field;
}

LCH_List *LCH_CSVParseRecord(const char *const csv, const size_t size) {
  assert(csv != NULL);

  LCH_CSVParser parser = {
      .cursor = csv,
      .end = csv + size,
      .row = 1,
      .column = 1,
  };

  LCH_List *const record = ParseRecord(&parser);
  if (record == NULL) {
    return NULL;
  }
  return record;
}

LCH_List *LCH_CSVParseTable(const char *str, const size_t size) {
  assert(str != NULL);

  LCH_CSVParser parser = {
      .cursor = str,
      .end = str + size,
      .row = 1,
      .column = 1,
  };

  LCH_List *const table = ParseTable(&parser);
  return table;
}

LCH_List *LCH_CSVParseFile(const char *const path) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return NULL;
  }

  if (!LCH_BufferReadFile(buffer, path)) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  const char *const csv = LCH_BufferData(buffer);
  const size_t size = LCH_BufferLength(buffer);

  LCH_List *table = LCH_CSVParseTable(csv, size);
  if (table == NULL) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  LCH_BufferDestroy(buffer);
  return table;
}

static bool ComposeField(LCH_Buffer *const csv, const char *const raw,
                         const size_t size) {
  assert(csv != NULL);
  assert(raw != NULL);

  LCH_Buffer *temp = LCH_BufferCreate();
  if (temp == NULL) {
    return false;
  }

  /* Fields starting with or ending with a space should be escaped */
  bool escape = size > 0 && (raw[0] == ' ' || raw[size - 1] == ' ');

  for (size_t i = 0; i < size; i++) {
    if (!TEXTDATA(raw[i])) {
      escape = true;
      if (raw[i] == '"') {
        if (!LCH_BufferPrintFormat(temp, "\"\"")) {
          LCH_BufferDestroy(temp);
          return false;
        }
        continue;
      }
    }
    if (!LCH_BufferAppend(temp, raw[i])) {
      LCH_BufferDestroy(temp);
      return false;
    }
  }

  if (escape) {
    if (!LCH_BufferAppend(csv, '"')) {
      LCH_BufferDestroy(temp);
      return false;
    }
  }

  if (!LCH_BufferAppendBuffer(csv, temp)) {
    LCH_BufferDestroy(temp);
    return false;
  }
  LCH_BufferDestroy(temp);

  if (escape) {
    if (!LCH_BufferAppend(csv, '"')) {
      return false;
    }
  }

  return true;
}

bool LCH_CSVComposeField(LCH_Buffer **const _csv, const char *const raw,
                         const size_t size) {
  const bool create_buffer = *_csv == NULL;
  LCH_Buffer *const csv = (create_buffer) ? LCH_BufferCreate() : *_csv;
  if (csv == NULL) {
    return false;
  }

  const size_t offset = LCH_BufferLength(csv);
  if (!ComposeField(csv, raw, size)) {
    if (create_buffer) {
      LCH_BufferChop(csv, offset);
    } else {
      LCH_BufferDestroy(csv);
    }
    return NULL;
  }

  *_csv = csv;
  return true;
}

static bool ComposeRecord(LCH_Buffer *const csv, const LCH_List *const record) {
  assert(csv != NULL);
  assert(record != NULL);

  const size_t num_fields = LCH_ListLength(record);
  for (size_t i = 0; i < num_fields; i++) {
    if (i > 0) {
      if (!LCH_BufferAppend(csv, ',')) {
        return false;
      }
    }
    LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, i);
    const char *const raw = LCH_BufferData(field);
    const size_t length = LCH_BufferLength(field);
    if (!ComposeField(csv, raw, length)) {
      return false;
    }
  }

  return true;
}

bool LCH_CSVComposeRecord(LCH_Buffer **const _csv,
                          const LCH_List *const record) {
  assert(record != NULL);

  const bool create_buffer = *_csv == NULL;
  LCH_Buffer *const csv = (create_buffer) ? LCH_BufferCreate() : *_csv;
  if (csv == NULL) {
    return false;
  }
  const size_t offset = LCH_BufferLength(csv);

  if (!ComposeRecord(csv, record)) {
    if (create_buffer) {
      LCH_BufferDestroy(csv);
    } else {
      // If we fuck up, we want to leave the buffer the same way we found it
      LCH_BufferChop(csv, offset);
    }
    return false;
  }

  *_csv = csv;
  return true;
}

bool LCH_CSVComposeTable(LCH_Buffer **const _csv, const LCH_List *const table) {
  assert(table != NULL);
  assert(_csv != NULL);

  const bool create_buffer = *_csv == NULL;
  LCH_Buffer *const csv = (create_buffer) ? LCH_BufferCreate() : *_csv;
  if (csv == NULL) {
    return false;
  }
  const size_t offset = LCH_BufferLength(csv);

  const size_t length = LCH_ListLength(table);
  for (size_t i = 0; i < length; i++) {
    if (i > 0) {
      if (!LCH_BufferPrintFormat(csv, "\r\n")) {
        if (create_buffer) {
          LCH_BufferDestroy(csv);
        } else {
          LCH_BufferChop(csv, offset);
        }
        return false;
      }
    }

    LCH_List *record = (LCH_List *)LCH_ListGet(table, i);
    if (!ComposeRecord(csv, record)) {
      if (create_buffer) {
        LCH_BufferDestroy(csv);
      } else {
        LCH_BufferChop(csv, offset);
      }
      return false;
    }
  }

  *_csv = csv;
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
