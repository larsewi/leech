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

#define COMMA "COMMA"
#define CARRIAGE_RETURN "CARRIAGE_RETURN"
#define DOUBLE_QUOTE "DOUBLE_QUOTE"
#define LINE_FEED "LINE_FEED"
#define TEXT_DATA "TEXT_DATA"

typedef struct Token {
  char *type;
  char *data;
} Token;

static bool lex(const LCH_Instance *instance, const char *buffer, long size,
                Token *tokens, long *nTokens);

bool LCH_TableReadCallbackCSV(const LCH_Instance *instance,
                              const char *filename, char ****table) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    LCH_LOG_ERROR(instance, "fopen: %s", strerror(errno));
    return false;
  }

  if (fseek(file, 0, SEEK_END) < 0) {
    LCH_LOG_ERROR(instance, "fseek: %s", strerror(errno));
    return false;
  }

  const long size = ftell(file);
  if (size < 0) {
    LCH_LOG_ERROR(instance, "ftell: %s", strerror(errno));
    return false;
  }

  if (fseek(file, 0, SEEK_SET) < 0) {
    LCH_LOG_ERROR(instance, "fseek: %s", strerror(errno));
    return false;
  }

  char *buffer = malloc(size + 1);
  if (buffer == NULL) {
    LCH_LOG_ERROR(instance, "malloc: %s", strerror(errno));
    return NULL;
  }
  buffer[size] = '\0';

  if (fread(buffer, 1, size, file) != size) {
    LCH_LOG_ERROR(instance, "fread: %s", strerror(errno));
    return false;
  }
  LCH_LOG_DEBUG(instance, "Read '%s' (%d Byte%s) with content:\n%s", filename,
                size, (size == 1) ? "" : "s", buffer);

  long nTokens;
  if (!lex(instance, buffer, size, NULL, &nTokens)) {
    LCH_LOG_ERROR(instance, "Failed to count tokens");
    return false;
  }
  LCH_LOG_DEBUG(instance, "Counted %d tokens", nTokens);

  Token tokens[nTokens];
  if (!lex(instance, buffer, size, tokens, &nTokens)) {
    LCH_LOG_ERROR(instance, "Failed to load tokens");
    return false;
  }
  LCH_LOG_DEBUG(instance, "Loaded %d tokens", nTokens);

  return true;
}

bool LCH_TableWriteCallbackCSV(const LCH_Instance *instance,
                               const char *filename, char ****table) {}

static bool t_COMMA(const char *buffer, long l, long r) {
  if (r - l != 1) {
    return false;
  }
  return buffer[l] == 0x2C;
}

static bool isCarriageReturn(const char *buffer, long l, long r) {
  if (r - l != 1) {
    return false;
  }
  return buffer[l] == 0x0D;
}

static bool isDoubleQuote(const char *buffer, long l, long r) {
  if (r - l != 1) {
    return false;
  }
  return buffer[l] == 0x22;
}

static bool isLineFeed(const char *buffer, long l, long r) {
  if (r - l != 1) {
    return false;
  }
  return buffer[l] == 0x0A;
}

static bool isTextData(const char *buffer, long l, long r) {
  for (long i = l; i < r; i++) {
    char ch = buffer[i];
    if (!((ch >= 0x20 && ch <= 0x21) || (ch >= 0x23 && ch <= 0x2B) ||
          (ch >= 0x2D && ch <= 0x7E))) {
      return false;
    }
  }
  return true;
}

static bool lex(const LCH_Instance *instance, const char *buffer, long size,
                Token *tokens, long *nTokens) {
  long i = 0;
  long l = 0;
  long r = size;
  char *type = NULL;
  char *data = NULL;

  while (l < r) {
    if (t_COMMA(buffer, l, r)) {
      if ((type = strdup(COMMA)) == NULL) {
        LCH_LOG_ERROR(instance, "strdup: %s", strerror(errno));
        return false;
      }
    } else if (isCarriageReturn(buffer, l, r)) {
      if ((type = strdup(CARRIAGE_RETURN)) == NULL) {
        LCH_LOG_ERROR(instance, "strdup: %s", strerror(errno));
        return false;
      }
    } else if (isDoubleQuote(buffer, l, r)) {
      if ((type = strdup(DOUBLE_QUOTE)) == NULL) {
        LCH_LOG_ERROR(instance, "strdup: %s", strerror(errno));
        return false;
      }
    } else if (isLineFeed(buffer, l, r)) {
      if ((type = strdup(LINE_FEED)) == NULL) {
        LCH_LOG_ERROR(instance, "strdup: %s", strerror(errno));
        return false;
      }
    } else if (isTextData(buffer, l, r)) {
      if ((type = strdup(TEXT_DATA)) == NULL) {
        LCH_LOG_ERROR(instance, "strdup: %s", strerror(errno));
        return false;
      }
    } else {
      r -= 1;
      continue;
    }
    assert(type != NULL);

    if ((data = strndup(buffer + l, r - l)) == NULL) {
      LCH_LOG_ERROR(instance, "strndup: %s", strerror(errno));
      free(type);
      return false;
    }

    LCH_LOG_DEBUG(instance, "Token '%s' [%ld:%ld]: '%s'", type, l, r, data);
    if (tokens != NULL) {
      tokens[i].type = type;
      tokens[i].data = data;
    } else {
      free(type);
      free(data);
    }

    i += 1;
    l = r;
    r = size;
  }

  if (l != size) {
    LCH_LOG_ERROR(instance, "Illegal token at %d", l);
    return false;
  }
  *nTokens = i;
  return true;
}

static void p_non_escaped(void) {
  // non_escaped = *TEXTDATA
}

static void p_escaped(void) {
  // escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE
}

static void p_field(void) {
  // field = (escaped / non-escaped)
}

static void p_name(void) {
  // name = field
}

static void p_record(void) {
  // record = field *(COMMA field)
}

static void p_header(void) {
  // header = name *(COMMA name)
}

static void p_file(void) {
  // file = [header CRLF] record * (CRLF record) [CRLF]
}
