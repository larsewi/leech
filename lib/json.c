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
  double number;
  char *str;
  LCH_List *array;
  LCH_Dict *object;
};

static const char *const LCH_JSON_TYPE_TO_STRING[] = {
    "null", "true", "false", "string", "number", "array", "object"};

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

const char *LCH_JsonGetString(const LCH_Json *const json) {
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

static bool StringComposeString(const char *const str,
                                LCH_Buffer *const buffer) {
  assert(str != NULL);
  assert(buffer != NULL);

  if (!LCH_BufferAppend(buffer, '"')) {
    return false;
  }

  for (const char *ch = str; *ch != '\0'; ch++) {
    const char *control_sequence = NULL;
    switch (*ch) {
      case '"':
        control_sequence = "\\\"";
        break;
      case '\\':
        control_sequence = "\\\\";
        break;
      case '\b':
        control_sequence = "\\b";
        break;
      case '\f':
        control_sequence = "\\f";
        break;
      case '\n':
        control_sequence = "\\n";
        break;
      case '\r':
        control_sequence = "\\r";
        break;
      case '\t':
        control_sequence = "\\t";
        break;
      default:
        if (!LCH_BufferAppend(buffer, *ch)) {
          return false;
        }
        continue;
    }
    assert(control_sequence != NULL);
    if (!LCH_BufferPrintFormat(buffer, control_sequence)) {
      return false;
    }
  }
  if (!LCH_BufferAppend(buffer, '"')) {
    return false;
  }
  return true;
}

static bool JsonComposeString(const LCH_Json *const json,
                              LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_STRING);
  assert(json->str != NULL);

  if (!StringComposeString(json->str, buffer)) {
    return false;
  }

  return true;
}

static LCH_Json *JsonStringCopy(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_STRING);
  assert(json->str != NULL);

  char *const dup = strdup(json->str);
  if (dup == NULL) {
    LCH_LOG_ERROR("Failed to allocate memeory for JSON structure: %s",
                  strerror(errno));
    return NULL;
  }

  LCH_Json *copy = LCH_JsonStringCreate(dup);
  return copy;
}

static bool JsonStringEqual(const LCH_Json *const a, const LCH_Json *const b) {
  assert(a != NULL);
  assert(a->type == LCH_JSON_TYPE_STRING);
  assert(a->str != NULL);

  assert(b != NULL);
  assert(b->type == LCH_JSON_TYPE_STRING);
  assert(b->str != NULL);

  const bool equal = LCH_StringEqual(a->str, b->str);
  return equal;
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

bool LCH_JsonObjectHasKey(const LCH_Json *const json, const char *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  assert(key != NULL);
  const bool present = LCH_DictHasKey(json->object, key);
  return present;
}

const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const char *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  const LCH_Json *const value = (LCH_Json *)LCH_DictGet(json->object, key);
  return value;
}

const char *LCH_JsonObjectGetString(const LCH_Json *const json,
                                    const char *const key) {
  assert(json != NULL);
  assert(key != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);

  if (!LCH_JsonObjectHasKey(json, key)) {
    LCH_LOG_ERROR(
        "Failed to get value using key '%s': "
        "Key does not exist.",
        key);
    return NULL;
  }

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  if (child->type != LCH_JSON_TYPE_STRING) {
    LCH_LOG_ERROR(
        "Failed to get value using key '%s': "
        "Expected type string, found type %s",
        key, LCH_JSON_TYPE_TO_STRING[child->type]);
    return NULL;
  }

  const char *const str = LCH_JsonGetString(child);
  return str;
}

const LCH_Json *LCH_JsonObjectGetObject(const LCH_Json *const json,
                                        const char *const key) {
  assert(json != NULL);
  assert(key != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);

  if (!LCH_JsonObjectHasKey(json, key)) {
    LCH_LOG_ERROR(
        "Failed to get value using key '%s': "
        "Key does not exist.",
        key);
    return NULL;
  }

  const LCH_Json *child = LCH_JsonObjectGet(json, key);
  if (child->type != LCH_JSON_TYPE_OBJECT) {
    LCH_LOG_ERROR(
        "Failed to get value using key '%s': "
        "Expected type object, found type %s.",
        key, LCH_JSON_TYPE_TO_STRING[child->type]);
    return NULL;
  }

  return child;
}

const LCH_Json *LCH_JsonObjectGetArray(const LCH_Json *const json,
                                       const char *const key) {
  assert(json != NULL);
  assert(key != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);

  if (!LCH_JsonObjectHasKey(json, key)) {
    LCH_LOG_ERROR(
        "Failed to get value using key '%s': "
        "Key does not exist.",
        key);
    return NULL;
  }

  const LCH_Json *child = LCH_JsonObjectGet(json, key);
  if (child->type != LCH_JSON_TYPE_ARRAY) {
    LCH_LOG_ERROR(
        "Failed to get value using key '%s': "
        "Expected type array, found type %s.",
        key, LCH_JSON_TYPE_TO_STRING[child->type]);
    return NULL;
  }

  return child;
}

bool LCH_JsonObjectSet(LCH_Json *const json, const char *const key,
                       LCH_Json *const value) {
  assert(json != NULL);
  assert(key != NULL);
  assert(value != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);
  const bool success = LCH_DictSet(json->object, key, value, LCH_JsonDestroy);
  return success;
}

bool LCH_JsonObjectSetString(LCH_Json *const json, const char *const key,
                             char *const str) {
  assert(json != NULL);
  assert(key != NULL);
  assert(str != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  LCH_Json *const value = LCH_JsonStringCreate(str);
  if (value == NULL) {
    return false;
  }

  if (!LCH_JsonObjectSet(json, key, value)) {
    LCH_JsonDestroy(value);
    return false;
  }

  return true;
}

bool LCH_JsonObjectSetStringDuplicate(LCH_Json *const json,
                                      const char *const key,
                                      const char *const str) {
  assert(json != NULL);
  assert(key != NULL);
  assert(str != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  char *const dup = LCH_StringDuplicate(str);
  if (dup == NULL) {
    return false;
  }

  if (!LCH_JsonObjectSetString(json, key, dup)) {
    free(dup);
    return false;
  }

  return true;
}

bool LCH_JsonObjectSetNumber(LCH_Json *const json, const char *const key,
                             const double number) {
  assert(json != NULL);
  assert(key != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  LCH_Json *const value = LCH_JsonNumberCreate(number);
  if (value == NULL) {
    return false;
  }

  if (!LCH_JsonObjectSet(json, key, value)) {
    LCH_JsonDestroy(value);
    return false;
  }

  return true;
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
    if (!StringComposeString(key, buffer)) {
      LCH_ListDestroy(keys);
      return false;
    }

    if (!LCH_BufferAppend(buffer, ':')) {
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

static LCH_Json *JsonObjectCopy(const LCH_Json *const object) {
  assert(object != NULL);
  assert(object->type == LCH_JSON_TYPE_OBJECT);

  LCH_Json *const object_copy = LCH_JsonObjectCreate();
  if (object_copy != NULL) {
    return NULL;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(object);
  if (keys == NULL) {
    LCH_JsonDestroy(object_copy);
    return NULL;
  }

  const size_t length = LCH_ListLength(keys);
  for (size_t i = 0; i < length; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    const LCH_Json *const value = LCH_JsonObjectGet(object, key);
    assert(value != NULL);

    LCH_Json *const value_copy = LCH_JsonCopy(value);
    if (value_copy == NULL) {
      LCH_ListDestroy(keys);
      LCH_JsonDestroy(object_copy);
      return NULL;
    }

    if (!LCH_JsonObjectSet(object_copy, key, value_copy)) {
      LCH_JsonDestroy(value_copy);
      LCH_ListDestroy(keys);
      LCH_JsonDestroy(object_copy);
      return NULL;
    }
  }

  LCH_JsonDestroy(keys);
  return object_copy;
}

static bool JsonObjectEqual(const LCH_Json *const a, const LCH_Json *const b) {
  assert(a != NULL);
  assert(a->type == LCH_JSON_TYPE_OBJECT);
  assert(a->object != NULL);

  assert(b != NULL);
  assert(b->type == LCH_JSON_TYPE_OBJECT);
  assert(b->object != NULL);

  const size_t length = LCH_JsonObjectLength(a);
  if (length != LCH_JsonObjectLength(b)) {
    return false;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(a);
  assert(length == LCH_ListLength(keys));

  for (size_t i = 0; i < length; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    if (!LCH_JsonObjectHasKey(b, key)) {
      LCH_ListDestroy(keys);
      return false;
    }
    assert(LCH_JsonObjectHasKey(a, key));

    const LCH_Json *const value_a = LCH_JsonObjectGet(a, key);
    assert(value_a != NULL);

    const LCH_Json *const value_b = LCH_JsonObjectGet(b, key);
    assert(value_b != NULL);

    if (!LCH_JsonEqual(value_a, value_b)) {
      LCH_ListDestroy(keys);
      return false;
    }
  }

  LCH_ListDestroy(keys);
  return true;
}

LCH_Json *LCH_JsonObjectKeysSetMinus(const LCH_Json *const a,
                                     const LCH_Json *const b) {
  assert(a != NULL);
  assert(LCH_JsonGetType(a) == LCH_JSON_TYPE_OBJECT);

  assert(b != NULL);
  assert(LCH_JsonGetType(b) == LCH_JSON_TYPE_OBJECT);

  LCH_Json *const result = LCH_JsonObjectCreate();
  if (result == NULL) {
    return NULL;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(a);
  if (keys == NULL) {
    LCH_JsonDestroy(result);
    return NULL;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    if (!LCH_JsonObjectHasKey(b, key)) {
      const LCH_Json *const value_a = LCH_JsonObjectGet(a, key);
      assert(value_a != NULL);

      LCH_Json *const copy = LCH_JsonCopy(value_a);
      if (copy == NULL) {
        LCH_ListDestroy(keys);
        LCH_JsonDestroy(result);
        return NULL;
      }

      if (!LCH_JsonObjectSet(result, key, copy)) {
        LCH_JsonDestroy(copy);
        LCH_ListDestroy(keys);
        LCH_JsonDestroy(result);
        return NULL;
      }
    }
  }

  LCH_ListDestroy(keys);
  return result;
}

LCH_Json *LCH_JsonObjectKeysSetIntersectAndValuesSetMinus(
    const LCH_Json *const a, const LCH_Json *const b) {
  assert(a != NULL);
  assert(a->type == LCH_JSON_TYPE_OBJECT);

  assert(b != NULL);
  assert(b->type == LCH_JSON_TYPE_OBJECT);

  LCH_Json *const result = LCH_JsonObjectCreate();
  if (result == NULL) {
    return NULL;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(a);
  if (keys == NULL) {
    LCH_JsonDestroy(result);
    return NULL;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    if (LCH_JsonObjectHasKey(b, key)) {
      const LCH_Json *const value_a = LCH_JsonObjectGet(a, key);
      assert(value_a != NULL);

      const LCH_Json *const value_b = LCH_JsonObjectGet(b, key);
      assert(value_b != NULL);

      if (!LCH_JsonEqual(value_a, value_b)) {
        LCH_Json *const copy = LCH_JsonCopy(value_a);
        if (copy == NULL) {
          LCH_ListDestroy(keys);
          LCH_JsonDestroy(result);
          return NULL;
        }

        if (!LCH_JsonObjectSet(result, key, copy)) {
          LCH_JsonDestroy(copy);
          LCH_ListDestroy(keys);
          LCH_JsonDestroy(result);
          return NULL;
        }
      }
    }
  }

  LCH_ListDestroy(keys);
  return result;
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

const char *LCH_JsonArrayGetString(const LCH_Json *const json,
                                   const size_t index) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);

  const size_t length = LCH_JsonArrayLength(json);
  if (index >= length) {
    LCH_LOG_ERROR(
        "Failed to get value using index %zu: "
        "Index out of bounds (%zu >= %zu)",
        index, index, length);
    return NULL;
  }

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  if (child->type != LCH_JSON_TYPE_STRING) {
    LCH_LOG_ERROR(
        "Failed to get value using index %zu: "
        "Expected type string, found type %s",
        index, LCH_JSON_TYPE_TO_STRING[child->type]);
    return NULL;
  }

  const char *const str = LCH_JsonGetString(child);
  return str;
}

bool LCH_JsonArrayAppend(const LCH_Json *const json, LCH_Json *const element) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);
  assert(element != NULL);
  const bool success = LCH_ListAppend(json->array, element, LCH_JsonDestroy);
  return success;
}

size_t LCH_JsonArrayLength(const LCH_Json *const json) {
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

static LCH_Json *JsonArrayCopy(const LCH_Json *const array) {
  assert(array != NULL);
  assert(array->type == LCH_JSON_TYPE_ARRAY);

  LCH_Json *const array_copy = LCH_JsonArrayCreate();
  if (array_copy == NULL) {
    return NULL;
  }

  const size_t length = LCH_JsonArrayLength(array);
  for (size_t i = 0; i < length; i++) {
    const LCH_Json *const element = LCH_JsonArrayGet(array, i);
    assert(element != NULL);

    LCH_Json *const element_copy = LCH_JsonCopy(element);
    if (element_copy == NULL) {
      LCH_JsonDestroy(array_copy);
      return NULL;
    }

    if (!LCH_JsonArrayAppend(array_copy, element_copy)) {
      LCH_JsonDestroy(element_copy);
      LCH_JsonDestroy(array_copy);
      return NULL;
    }
  }

  return array_copy;
}

static bool JsonArrayEqual(const LCH_Json *const a, const LCH_Json *const b) {
  assert(a != NULL);
  assert(a->type == LCH_JSON_TYPE_ARRAY);
  assert(a->array != NULL);

  assert(b != NULL);
  assert(b->type == LCH_JSON_TYPE_ARRAY);
  assert(b->array != NULL);

  const size_t length = LCH_JsonArrayLength(a);
  if (length != LCH_JsonArrayLength(b)) {
    return false;
  }

  for (size_t i = 0; i < length; i++) {
    const LCH_Json *const element_a = (LCH_Json *)LCH_JsonArrayGet(a, i);
    assert(element_a != NULL);

    const LCH_Json *const element_b = (LCH_Json *)LCH_JsonArrayGet(b, i);
    assert(element_b != NULL);

    if (!LCH_JsonEqual(element_a, element_b)) {
      return false;
    }
  }

  return true;
}

/****************************************************************************/

LCH_Json *LCH_JsonNumberCreate(const double number) {
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

double LCH_JsonGetNumber(const LCH_Json *const json) {
  assert(json->type == LCH_JSON_TYPE_NUMBER);
  return json->number;
}

static const char *JsonParseNumber(const char *const str, LCH_Json **json) {
  int n_chars;
  double number;
  int ret = sscanf(str, "%le%n", &number, &n_chars);
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

static LCH_Json *JsonNumberCopy(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_NUMBER);
  LCH_Json *copy = LCH_JsonNumberCreate(json->number);
  return copy;
}

static bool JsonNumberEqual(const LCH_Json *const a, const LCH_Json *const b) {
  assert(a != NULL);
  assert(a->type == LCH_JSON_TYPE_NUMBER);

  assert(b != NULL);
  assert(b->type == LCH_JSON_TYPE_NUMBER);

  const bool equal = a->number == b->number;
  return equal;
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

LCH_Json *LCH_JsonCopy(const LCH_Json *const json) {
  assert(json != NULL);

  LCH_JsonType type = LCH_JsonGetType(json);
  switch (type) {
    case LCH_JSON_TYPE_NULL:
      return LCH_JsonNullCreate();

    case LCH_JSON_TYPE_TRUE:
      return LCH_JsonTrueCreate();

    case LCH_JSON_TYPE_FALSE:
      return LCH_JsonFalseCreate();

    case LCH_JSON_TYPE_STRING:
      return JsonStringCopy(json);

    case LCH_JSON_TYPE_NUMBER:
      return JsonNumberCopy(json);

    case LCH_JSON_TYPE_ARRAY:
      return JsonArrayCopy(json);

    case LCH_JSON_TYPE_OBJECT:
      return JsonObjectCopy(json);

    default:
      assert(false);
      LCH_LOG_ERROR("Failed to copy JSON: Illegal type %d", type);
      return NULL;
  }
}

bool LCH_JsonEqual(const LCH_Json *const a, const LCH_Json *b) {
  assert(a != NULL);
  assert(b != NULL);

  if (a->type != b->type) {
    return false;
  }

  switch (a->type) {
    case LCH_JSON_TYPE_NULL:
      // fallthrough

    case LCH_JSON_TYPE_TRUE:
      // fallthrough

    case LCH_JSON_TYPE_FALSE:
      return true;

    case LCH_JSON_TYPE_STRING:
      return JsonStringEqual(a, b);

    case LCH_JSON_TYPE_NUMBER:
      return JsonNumberEqual(a, b);

    case LCH_JSON_TYPE_ARRAY:
      return JsonArrayEqual(a, b);

    case LCH_JSON_TYPE_OBJECT:
      return JsonObjectEqual(a, b);

    default:
      assert(false);
      LCH_LOG_ERROR("Failed to copy JSON: Illegal type %d", a->type);
      return false;
  }
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
