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
  LCH_List *array;
  LCH_Dict *object;
};

static const char *JsonParse(const char *str, LCH_Json **json);
static bool JsonCompose(const LCH_Json *const json, LCH_Buffer *const buffer);

LCH_JsonType LCH_JsonGetType(const LCH_Json *const json) { return json->type; }

/****************************************************************************/

LCH_Json *LCH_JsonNullCreate() {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  (json)->type = LCH_JSON_TYPE_NULL;

  return json;
}

static const char *JsonParseNull(const char *const str, LCH_Json **json) {
  assert(str != NULL);
  assert(strncmp(str, "null", strlen("null")) == 0);

  LCH_Json *const tmp = LCH_JsonNullCreate();
  if (tmp == NULL) {
    return NULL;
  }
  *json = tmp;

  return str + strlen("null");
}

static bool JsonComposeNull(const LCH_Json *const json,
                            LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_NULL);
  return LCH_BufferPrintFormat(buffer, "null");
}

/****************************************************************************/

LCH_Json *LCH_JsonTrueCreate() {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  (json)->type = LCH_JSON_TYPE_TRUE;

  return json;
}

static const char *JsonParseTrue(const char *const str, LCH_Json **json) {
  assert(str != NULL);
  assert(strncmp(str, "true", strlen("true")) == 0);

  LCH_Json *const tmp = LCH_JsonTrueCreate();
  if (tmp == NULL) {
    return NULL;
  }
  *json = tmp;

  return str + strlen("true");
}

static bool JsonComposeTrue(const LCH_Json *const json,
                            LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_TRUE);
  return LCH_BufferPrintFormat(buffer, "true");
}

/****************************************************************************/

LCH_Json *LCH_JsonFalseCreate() {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  (json)->type = LCH_JSON_TYPE_FALSE;

  return json;
}

static const char *JsonParseFalse(const char *const str, LCH_Json **json) {
  assert(str != NULL);
  assert(strncmp(str, "false", strlen("false")) == 0);

  LCH_Json *const tmp = LCH_JsonFalseCreate();
  if (tmp == NULL) {
    return NULL;
  }
  *json = tmp;

  return str + strlen("false");
}

static bool JsonComposeFalse(const LCH_Json *const json,
                             LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_FALSE);
  return LCH_BufferPrintFormat(buffer, "false");
}

/****************************************************************************/

LCH_Json *LCH_JsonStringCreate(char *const str) {
  assert(str != NULL);

  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  json->type = LCH_JSON_TYPE_STRING;
  json->str = str;

  return json;
}

const char *LCH_JsonStringGet(const LCH_Json *const json) {
  assert(json->type == LCH_JSON_TYPE_STRING);
  assert(json->str != NULL);
  return json->str;
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
  char *value = LCH_BufferToString(buffer);

  LCH_Json *const tmp = LCH_JsonStringCreate(value);
  if (tmp == NULL) {
    free(value);
    return NULL;
  }
  *json = tmp;

  return str;
}

static bool JsonComposeString(const LCH_Json *const json,
                              LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_STRING);
  assert(json->str != NULL);
  return LCH_BufferPrintFormat(buffer, "\"%s\"", json->str);
}

/****************************************************************************/

LCH_Json *LCH_JsonObjectCreate() {
  LCH_Dict *const dict = LCH_DictCreate();
  if (dict == NULL) {
    return NULL;
  }

  LCH_Json *object = LCH_JsonObjectCreateFromDict(dict);
  return object;
}

LCH_Json *LCH_JsonObjectCreateFromDict(LCH_Dict *const dict) {
  assert(dict != NULL);

  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  json->type = LCH_JSON_TYPE_OBJECT;
  json->object = dict;

  return json;
}

LCH_List *LCH_JsonObjectGetKeys(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  LCH_List *const keys = LCH_DictGetKeys(json->object);
  return keys;
}

const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const char *const key) {
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  const LCH_Json *const value = (LCH_Json *)LCH_DictGet(json->object, key);
  return value;
}

bool LCH_JsonObjectSet(const LCH_Json *const json, const char *const key,
                       LCH_Json *const value) {
  assert(json != NULL);
  assert(key != NULL);
  assert(value != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  const bool success = LCH_DictSet(json->object, key, value, LCH_JsonDestroy);
  return success;
}

size_t LCH_JsonObjectLength(const LCH_Json *json) {
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  return LCH_DictLength(json->object);
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

    if (!LCH_DictSet(dict, key, value, LCH_JsonDestroy)) {
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

  LCH_Json *const object = LCH_JsonObjectCreateFromDict(dict);
  if (object == NULL) {
    LCH_DictDestroy(dict);
    return NULL;
  }
  *json = object;

  return str + 1;
}

static bool JsonComposeObject(const LCH_Json *const json,
                              LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  if (!LCH_BufferAppend(buffer, '{')) {
    return false;
  }

  LCH_List *const keys = LCH_DictGetKeys(json->object);
  if (keys == NULL) {
    return false;
  }

  const size_t length = LCH_ListLength(keys);
  for (size_t i = 0; i < length; i++) {
    if (i > 0) {
      if (!LCH_BufferAppend(buffer, ',')) {
        LCH_ListDestroy(keys);
        return false;
      }
    }

    const char *const key = (char *)LCH_ListGet(keys, i);
    if (!LCH_BufferPrintFormat(buffer, "\"%s\":", key)) {
      LCH_ListDestroy(keys);
      return false;
    }

    const LCH_Json *const element = LCH_JsonObjectGet(json, key);
    if (!JsonCompose(element, buffer)) {
      LCH_ListDestroy(keys);
      return false;
    }
  }
  LCH_ListDestroy(keys);

  if (!LCH_BufferAppend(buffer, '}')) {
    return false;
  }

  return true;
}

/****************************************************************************/

LCH_Json *LCH_JsonArrayCreate() {
  LCH_List *const list = LCH_ListCreate();
  if (list == NULL) {
    return NULL;
  }

  LCH_Json *array = LCH_JsonArrayCreateFromList(list);
  return array;
}

LCH_Json *LCH_JsonArrayCreateFromList(LCH_List *const list) {
  assert(list != NULL);

  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  json->type = LCH_JSON_TYPE_ARRAY;
  json->array = list;

  return json;
}

const LCH_Json *LCH_JsonArrayGet(const LCH_Json *const json,
                                 const size_t index) {
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);
  const LCH_Json *const value = (LCH_Json *)LCH_ListGet(json->array, index);
  return value;
}

bool LCH_JsonArrayAppend(const LCH_Json *const json, LCH_Json *const value) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  assert(value != NULL);
  const bool success = LCH_ListAppend(json->array, value, LCH_JsonDestroy);
  return success;
}

size_t LCH_JsonListLength(const LCH_Json *const json) {
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);
  return LCH_ListLength(json->array);
}

static const char *JsonParseArray(const char *str, LCH_Json **json) {
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

    if (!LCH_ListAppend(list, value, LCH_JsonDestroy)) {
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

  LCH_Json *const array = LCH_JsonArrayCreateFromList(list);
  if (array == NULL) {
    LCH_ListDestroy(list);
    return NULL;
  }
  *json = array;

  return str + 1;
}

static bool JsonComposeArray(const LCH_Json *const json,
                             LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);

  if (!LCH_BufferAppend(buffer, '[')) {
    return false;
  }

  const size_t length = LCH_ListLength(json->array);
  for (size_t i = 0; i < length; i++) {
    if (i > 0) {
      if (!LCH_BufferAppend(buffer, ',')) {
        return false;
      }
    }

    const LCH_Json *const element = LCH_JsonArrayGet(json, i);
    if (!JsonCompose(element, buffer)) {
      return false;
    }
  }

  if (!LCH_BufferAppend(buffer, ']')) {
    return false;
  }

  return true;
}

/****************************************************************************/

LCH_Json *LCH_JsonNumberCreate(const float number) {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON data structure: %s",
                  strerror(errno));
    return NULL;
  }
  json->type = LCH_JSON_TYPE_NUMBER;
  json->number = number;

  return json;
}

float LCH_JsonNumberGet(const LCH_Json *const json) {
  assert(json->type == LCH_JSON_TYPE_NUMBER);
  return json->number;
}

static const char *JsonParseNumber(const char *const str, LCH_Json **json) {
  int n_chars;
  float number;
  int ret = sscanf(str, "%e%n", &number, &n_chars);
  if (ret != 1) {
    LCH_LOG_ERROR(
        "Failed to parse JSON string: Syntax error; expected a number");
    return NULL;
  }

  LCH_Json *const tmp = LCH_JsonNumberCreate(number);
  if (tmp == NULL) {
    return NULL;
  }
  *json = tmp;

  return str + n_chars;
}

static bool JsonComposeNumber(const LCH_Json *const json,
                              LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_NUMBER);
  return LCH_BufferPrintFormat(buffer, "%g", json->number);
}

/****************************************************************************/

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
    return JsonParseArray(str, json);
  }
  if (isdigit(*str) != 0 || *str == '-') {
    return JsonParseNumber(str, json);
  } else {
    assert(false);
    LCH_LOG_ERROR(
        "Failed to parse JSON: Expected 'null', 'true', 'false', NUMBER, "
        "STRING, OBJECT, ARRAY; found '%c'",
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

static bool JsonCompose(const LCH_Json *const json, LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);

  LCH_JsonType type = LCH_JsonGetType(json);
  switch (type) {
    case LCH_JSON_TYPE_NULL:
      return JsonComposeNull(json, buffer);

    case LCH_JSON_TYPE_TRUE:
      return JsonComposeTrue(json, buffer);

    case LCH_JSON_TYPE_FALSE:
      return JsonComposeFalse(json, buffer);

    case LCH_JSON_TYPE_STRING:
      return JsonComposeString(json, buffer);

    case LCH_JSON_TYPE_NUMBER:
      return JsonComposeNumber(json, buffer);

    case LCH_JSON_TYPE_ARRAY:
      return JsonComposeArray(json, buffer);

    case LCH_JSON_TYPE_OBJECT:
      return JsonComposeObject(json, buffer);

    default:
      assert(false);
      LCH_LOG_ERROR("Failed to compose JSON: Illegal type %d", type);
      return false;
  }
}

char *LCH_JsonCompose(const LCH_Json *const json) {
  assert(json != NULL);

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return NULL;
  }

  if (!JsonCompose(json, buffer)) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  char *const str = LCH_BufferToString(buffer);
  return str;
}

void LCH_JsonDestroy(void *const self) {
  LCH_Json *const json = (LCH_Json *)self;
  if (json != NULL) {
    free(json->str);
    LCH_ListDestroy(json->array);
    LCH_DictDestroy(json->object);
  }
  free(json);
}
