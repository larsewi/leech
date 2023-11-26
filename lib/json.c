#include "json.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <memory.h>
#include <string.h>

#include "leech.h"
#include "utils.h"

struct LCH_Json {
  LCH_JsonType type;
  float number;
  char *str;
  LCH_List *list;
  LCH_Dict *object;
};

LCH_JsonType LCH_JsonGetType(const LCH_Json *const json) { return json->type; }

/****************************************************************************/

const char *LCH_JsonStringGet(const LCH_Json *const json) {
  assert(json->type == LCH_JSON_TYPE_STRING);
  assert(json->str != NULL);
  return json->str;
}

/****************************************************************************/

const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const char *const key) {
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  const LCH_Json *const value = LCH_DictGet(json->object, key);
  return value;
}

size_t LCH_JsonObjectLength(const LCH_Json *json) {
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  return LCH_DictLength(json->object);
}

/****************************************************************************/

const LCH_Json *LCH_JsonListGet(const LCH_Json *const json,
                                const size_t index) {
  assert(json->type == LCH_JSON_TYPE_LIST);
  assert(json->list != NULL);
  const LCH_Json *const value = LCH_ListGet(json->list, index);
  return value;
}

size_t LCH_JsonListLength(const LCH_Json *const json) {
  assert(json->type == LCH_JSON_TYPE_LIST);
  assert(json->list != NULL);
  return LCH_ListLength(json->list);
}

/****************************************************************************/

float LCH_JsonNumberGet(const LCH_Json *const json) {
  assert(json->type == LCH_JSON_TYPE_NUMBER);
  return json->number;
}

/****************************************************************************/

static const char *JsonParse(const char *str, LCH_Json **json);

static const char *JsonParseNull(const char *const str, LCH_Json **json) {
  assert(str != NULL);
  assert(strncmp(str, "null", strlen("null")) == 0);

  *json = calloc(1, sizeof(LCH_Json));
  if (*json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  (*json)->type = LCH_JSON_TYPE_NULL;

  return str + strlen("null");
}

static const char *JsonParseTrue(const char *const str, LCH_Json **json) {
  assert(str != NULL);
  assert(strncmp(str, "true", strlen("true")) == 0);

  *json = calloc(1, sizeof(LCH_Json));
  if (*json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  (*json)->type = LCH_JSON_TYPE_TRUE;

  return str + strlen("true");
}

static const char *JsonParseFalse(const char *const str, LCH_Json **json) {
  assert(str != NULL);
  assert(strncmp(str, "false", strlen("false")) == 0);

  *json = calloc(1, sizeof(LCH_Json));
  if (*json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  (*json)->type = LCH_JSON_TYPE_FALSE;

  return str + strlen("false");
}

static const char *BufferParseString(const char *str, LCH_Buffer **buffer) {
  assert(str != NULL);
  assert(*str == '"');

  str++;  // Skip initial double quote

  *buffer = LCH_BufferCreate();
  if (*buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for JSON string");
    return NULL;
  }

  while (*str != '\0' && *str != '"') {
    if (*str == '\\') {
      switch (str[1]) {
        case '"':
          if (!LCH_BufferAppend(*buffer, '"')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case '\\':
          if (!LCH_BufferAppend(*buffer, '\\')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case '/':
          if (!LCH_BufferAppend(*buffer, '/')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case 'b':
          if (!LCH_BufferAppend(*buffer, '\b')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case 'f':
          if (!LCH_BufferAppend(*buffer, '\f')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case 'n':
          if (!LCH_BufferAppend(*buffer, '\n')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case 'r':
          if (!LCH_BufferAppend(*buffer, '\r')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case 't':
          if (!LCH_BufferAppend(*buffer, '\t')) {
            LCH_BufferDestroy(*buffer);
            return NULL;
          }
          break;

        case 'u':
          if (!LCH_BufferUnicodeToUTF8(*buffer, str + 2)) {
            LCH_LOG_ERROR(
                "Failed to parse JSON string: Illegal unicode control sequence "
                "'%.6s'",
                str);
            LCH_BufferDestroy(*buffer);
          }
          str += 4;
          break;

        default:
          LCH_LOG_ERROR(
              "Failed to parse JSON string: Illegal control character '\\%c'",
              *str);
          LCH_BufferDestroy(*buffer);
          return NULL;
      }
      str++;
    } else if (!LCH_BufferAppend(*buffer, str[0])) {
      LCH_BufferDestroy(*buffer);
      return NULL;
    }
    str++;
  }

  if (*str != '"') {
    LCH_LOG_ERROR(
        "Failed to parse JSON string: Syntax error; expected '\"', found '%c'",
        *str);
    LCH_BufferDestroy(*buffer);
    return NULL;
  }

  return str + 1;
}

static const char *JsonParseString(const char *str, LCH_Json **json) {
  assert(str != NULL);
  assert(*str == '"');

  LCH_Buffer *buffer;
  str = BufferParseString(str, &buffer);
  if (str == NULL) {
    return NULL;
  }

  *json = calloc(1, sizeof(LCH_Json));
  if (*json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    LCH_BufferDestroy(buffer);
    return NULL;
  }
  (*json)->type = LCH_JSON_TYPE_STRING;
  (*json)->str = LCH_BufferToString(buffer);

  return str;
}

static const char *JsonParseObject(const char *str, LCH_Json **json) {
  assert(str != NULL);
  assert(*str == '{');

  LCH_Dict *dict = LCH_DictCreate();
  if (dict == NULL) {
    return NULL;
  }

  // Skip initial curly brace
  str++;

  // Skip whitespace
  str += strspn(str, " \r\n\t");

  bool first = true;
  while (*str != '\0' && *str != '}') {
    if (!first) {
      // Skip comma
      if (*str != ',') {
        LCH_LOG_ERROR(
            "Failed to parse JSON: Syntax error; expected ',', found '%c'",
            *str);
        LCH_DictDestroy(dict);
        return NULL;
      }
      str++;

      // Skip whitespace
      str += strspn(str, " \r\n\t");
    }
    first = false;

    // Extract key
    LCH_Buffer *buffer;
    str = BufferParseString(str, &buffer);
    if (str == NULL) {
      LCH_DictDestroy(dict);
      return NULL;
    }
    char *const key = LCH_BufferToString(buffer);

    // Skip whitespace
    str += strspn(str, " \r\n\t");

    // Skip colon
    if (*str != ':') {
      LCH_LOG_ERROR(
          "Failed to parse JSON: Syntax error; expected ':', found '%c'", *str);
      free(key);
      LCH_DictDestroy(dict);
      return NULL;
    }
    str++;

    // Extract value
    LCH_Json *value;
    str = JsonParse(str, &value);
    if (str == NULL) {
      free(key);
      LCH_DictDestroy(dict);
    }

    if (!LCH_DictSet(dict, key, value, (void (*)(void *))LCH_JsonDestroy)) {
      free(key);
      LCH_DictDestroy(dict);
      LCH_JsonDestroy(value);
      return NULL;
    }

    free(key);

    // Skip whitespace
    str += strspn(str, " \r\n\t");
  }

  if (*str != '}') {
    LCH_LOG_ERROR(
        "Failed to parse JSON string: Syntax error; expected '}', found '%c'",
        *str);
    LCH_DictDestroy(dict);
    return NULL;
  }

  *json = calloc(1, sizeof(LCH_Json));
  if (*json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    LCH_DictDestroy(dict);
    return NULL;
  }
  (*json)->type = LCH_JSON_TYPE_OBJECT;
  (*json)->object = dict;

  return str + 1;
}

static const char *JsonParseList(const char *str, LCH_Json **json) {
  assert(str != NULL);
  assert(*str == '[');

  LCH_List *list = LCH_ListCreate();
  if (list == NULL) {
    return NULL;
  }

  // Skip initial square bracket
  str++;

  // Skip whitespace
  str += strspn(str, " \r\n\t");

  bool first = true;
  while (*str != '\0' && *str != ']') {
    if (!first) {
      // Skip comma
      if (*str != ',') {
        LCH_LOG_ERROR(
            "Failed to parse JSON: Syntax error; expected ',', found '%c'",
            *str);
        LCH_ListDestroy(list);
        return NULL;
      }
      str++;

      // Skip whitespace
      str += strspn(str, " \r\n\t");
    }
    first = false;

    // Extract value
    LCH_Json *value;
    str = JsonParse(str, &value);
    if (str == NULL) {
      LCH_ListDestroy(list);
      return NULL;
    }

    if (!LCH_ListAppend(list, value, (void (*)(void *))LCH_JsonDestroy)) {
      LCH_JsonDestroy(value);
      LCH_ListDestroy(list);
      return NULL;
    }

    // Skip whitespace
    str += strspn(str, " \r\n\t");
  }

  if (*str != ']') {
    LCH_LOG_ERROR(
        "Failed to parse JSON string: Syntax error; expected ']', found '%c'",
        *str);
    LCH_ListDestroy(list);
    return NULL;
  }

  *json = calloc(1, sizeof(LCH_Json));
  if (*json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    LCH_ListDestroy(list);
    return NULL;
  }
  (*json)->type = LCH_JSON_TYPE_LIST;
  (*json)->list = list;

  return str + 1;
}

static const char *JsonParseNumber(const char *const str, LCH_Json **json) {
  int n_chars;
  float number;
  int ret = sscanf(str, "%e%n", &number, &n_chars);
  if (ret != 1) {
    LCH_LOG_ERROR(
        "Failed to parse JSON string: Syntax error; expected a number in the "
        "format [-]d.ddde±dd");
    return NULL;
  }

  *json = calloc(1, sizeof(LCH_Json));
  if (*json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  (*json)->type = LCH_JSON_TYPE_NUMBER;
  (*json)->number = number;

  return str + n_chars;
}

static const char *JsonParse(const char *str, LCH_Json **json) {
  assert(str != NULL);

  str += strspn(str, " \r\n\t");  // Skip whitespace

  if (strncmp(str, "null", strlen("null")) == 0) {
    return JsonParseNull(str, json);
  }
  if (strncmp(str, "true", strlen("true")) == 0) {
    return JsonParseTrue(str, json);
  }
  if (strncmp(str, "false", strlen("false")) == 0) {
    return JsonParseFalse(str, json);
  }
  if (*str == '"') {
    return JsonParseString(str, json);
  }
  if (*str == '{') {
    return JsonParseObject(str, json);
  }
  if (*str == '[') {
    return JsonParseList(str, json);
  }
  if (isdigit(*str) != 0 || *str == '-') {
    return JsonParseNumber(str, json);
  } else {
    LCH_LOG_ERROR(
        "Failed to parse JSON: Expected 'null', 'true', 'false', NUMBER, "
        "STRING, OBJECT, LIST; found '%c'",
        *str);
    return NULL;
  }
}

LCH_Json *LCH_JsonParse(const char *const str) {
  assert(str != NULL);

  LCH_Json *json;
  const char *const ret = JsonParse(str, &json);
  return (ret == NULL) ? NULL : json;
}

/****************************************************************************/

void LCH_JsonDestroy(LCH_Json *const json) {
  if (json != NULL) {
    free(json->str);
    LCH_ListDestroy(json->list);
    LCH_DictDestroy(json->object);
  }
  free(json);
}
