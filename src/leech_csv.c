/**
 * CSV Standard rfc4180
 * https://datatracker.ietf.org/doc/html/rfc4180
 */

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leech_csv.h"

#define IS_COMMA(ch) (ch == 0x2C)
#define IS_CARRIAGE_RETURN(ch) (ch == 0x0D)
#define IS_DOUBLE_QUOTE(ch) (ch == 0x22)
#define IS_LINE_FEED(ch) (ch == 0x0A)
#define IS_TEXT_DATA(ch)                                                       \
  ((ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) ||                 \
   (ch >= 0x2D && ch <= 0x7E));

bool LCH_TableReadCallbackCSV() {
  // FILE *file = fopen(filename, "r");
  // if (file == NULL) {
  //   LCH_LOG_ERROR(instance, "fopen: %s", strerror(errno));
  //   return false;
  // }

  // if (fseek(file, 0, SEEK_END) < 0) {
  //   LCH_LOG_ERROR(instance, "fseek: %s", strerror(errno));
  //   return false;
  // }

  // const long size = ftell(file);
  // if (size < 0) {
  //   LCH_LOG_ERROR(instance, "ftell: %s", strerror(errno));
  //   return false;
  // }

  // if (fseek(file, 0, SEEK_SET) < 0) {
  //   LCH_LOG_ERROR(instance, "fseek: %s", strerror(errno));
  //   return false;
  // }

  // char *buffer = malloc(size + 1);
  // if (buffer == NULL) {
  //   LCH_LOG_ERROR(instance, "malloc: %s", strerror(errno));
  //   return NULL;
  // }
  // buffer[size] = '\0';

  // if (fread(buffer, 1, size, file) != size) {
  //   LCH_LOG_ERROR(instance, "fread: %s", strerror(errno));
  //   return false;
  // }
  // LCH_LOG_DEBUG(instance, "Read '%s' (%d Byte%s) with content:\n%s",
  // filename,
  //               size, (size == 1) ? "" : "s", buffer);

  return true;
}

bool LCH_TableWriteCallbackCSV() { return true; }

static bool isComma(const char *buffer, long at) { return buffer[at] == 0x2C; }

static bool isCarriageReturn(const char *buffer, long at) {
  return buffer[at] == 0x0D;
}

static bool isDoubleQuote(const char *buffer, long at) {
  return buffer[at] == 0x22;
}

static bool isLineFeed(const char *buffer, long at) {
  return buffer[at] == 0x0A;
}

static bool isTextData(const char *buffer, long at) {
  char ch = buffer[at];
  return ((ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) ||
          (ch >= 0x2D && ch <= 0x7E));
}

static long ParseNonEscaped(const char *buffer, long size, long l) {
  // non_escaped = *TEXTDATA
  long r = l;
  while
    IS_TEXT_DATA(buffer[r]) { r += 1; }
  return r;
}

static void ParseEscaped(void) {
  // escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE
}

static void ParseField(void) {
  // field = (escaped / non-escaped)
}

static void ParseRecord(void) {
  // record = field *(COMMA field)
}

static void ParseHeader(void) {
  // header = field *(COMMA field)
}

static void ParseTable(const char *buffer, long size) {
  // table = header CRLF record * (CRLF record) [CRLF]
}
