#include "json.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "logger.h"
#include "utils.h"

struct LCH_Json {
  LCH_JsonType type;
  double number;
  char *str;
  LCH_List *array;
  LCH_Dict *object;
};

typedef struct {
  const char *cursor;
  const char *const end;
  LCH_List *const path;
} Parser;

/****************************************************************************/

static const char *const LCH_JSON_TYPE_TO_STRING[] = {
    "null", "true", "false", "string", "number", "array", "object"};

LCH_JsonType LCH_JsonGetType(const LCH_Json *const json) { return json->type; }

const char *LCH_JsonGetTypeAsString(const LCH_Json *const json) {
  assert(json != NULL);
  return LCH_JSON_TYPE_TO_STRING[json->type];
}

/****************************************************************************/

bool LCH_JsonIsNull(const LCH_Json *const json) {
  assert(json != NULL);

  const bool is_null = json->type == LCH_JSON_TYPE_NULL;
  return is_null;
}

bool LCH_JsonIsTrue(const LCH_Json *const json) {
  assert(json != NULL);

  const bool is_true = json->type == LCH_JSON_TYPE_TRUE;
  return is_true;
}

bool LCH_JsonIsFalse(const LCH_Json *const json) {
  assert(json != NULL);

  const bool is_false = json->type == LCH_JSON_TYPE_FALSE;
  return is_false;
}

bool LCH_JsonIsString(const LCH_Json *const json) {
  assert(json != NULL);

  const bool is_string = json->type == LCH_JSON_TYPE_STRING;
  return is_string;
}

bool LCH_JsonIsNumber(const LCH_Json *const json) {
  assert(json != NULL);

  const bool is_number = json->type == LCH_JSON_TYPE_NUMBER;
  return is_number;
}

bool LCH_JsonIsObject(const LCH_Json *const json) {
  assert(json != NULL);

  const bool is_object = json->type == LCH_JSON_TYPE_OBJECT;
  return is_object;
}

bool LCH_JsonIsArray(const LCH_Json *const json) {
  assert(json != NULL);

  const bool is_array = json->type == LCH_JSON_TYPE_ARRAY;
  return is_array;
}

/****************************************************************************/

bool LCH_JsonObjectChildIsNull(const LCH_Json *const json,
                               const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  assert(child != NULL);

  const bool is_null = LCH_JsonIsNull(child);
  return is_null;
}

bool LCH_JsonObjectChildIsTrue(const LCH_Json *const json,
                               const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  assert(child != NULL);

  const bool is_true = LCH_JsonIsTrue(child);
  return is_true;
}

bool LCH_JsonObjectChildIsFalse(const LCH_Json *const json,
                                const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  assert(child != NULL);

  const bool is_false = LCH_JsonIsFalse(child);
  return is_false;
}

bool LCH_JsonObjectChildIsString(const LCH_Json *const json,
                                 const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  assert(child != NULL);

  const bool is_string = LCH_JsonIsString(child);
  return is_string;
}

bool LCH_JsonObjectChildIsNumber(const LCH_Json *json, const LCH_Buffer *key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  assert(child != NULL);

  const bool is_number = LCH_JsonIsNumber(child);
  return is_number;
}

bool LCH_JsonObjectChildIsObject(const LCH_Json *const json,
                                 const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  assert(child != NULL);

  const bool is_object = LCH_JsonIsObject(child);
  return is_object;
}

bool LCH_JsonObjectChildIsArray(const LCH_Json *const json,
                                const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  assert(child != NULL);

  const bool is_array = LCH_JsonIsArray(child);
  return is_array;
}

/****************************************************************************/

bool LCH_JsonArrayChildIsNull(const LCH_Json *const json, const size_t index) {
  assert(json != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  assert(child != NULL);

  const bool is_null = LCH_JsonIsNull(child);
  return is_null;
}

bool LCH_JsonArrayChildIsTrue(const LCH_Json *const json, const size_t index) {
  assert(json != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  assert(child != NULL);

  const bool is_true = LCH_JsonIsTrue(child);
  return is_true;
}

bool LCH_JsonArrayChildIsFalse(const LCH_Json *const json, const size_t index) {
  assert(json != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  assert(child != NULL);

  const bool is_false = LCH_JsonIsFalse(child);
  return is_false;
}

bool LCH_JsonArrayChildIsString(const LCH_Json *const json,
                                const size_t index) {
  assert(json != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  assert(child != NULL);

  const bool is_string = LCH_JsonIsString(child);
  return is_string;
}

bool LCH_JsonArrayChildIsNumber(const LCH_Json *const json,
                                const size_t index) {
  assert(json != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  assert(child != NULL);

  const bool is_number = LCH_JsonIsNumber(child);
  return is_number;
}

bool LCH_JsonArrayChildIsObject(const LCH_Json *const json,
                                const size_t index) {
  assert(json != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  assert(child != NULL);

  const bool is_object = LCH_JsonIsObject(child);
  return is_object;
}

bool LCH_JsonArrayChildIsArray(const LCH_Json *const json, const size_t index) {
  assert(json != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  assert(child != NULL);

  const bool is_array = LCH_JsonIsArray(child);
  return is_array;
}

/****************************************************************************/

LCH_Json *LCH_JsonNullCreate() {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }

  json->type = LCH_JSON_TYPE_NULL;
  return json;
}

LCH_Json *LCH_JsonTrueCreate() {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }

  json->type = LCH_JSON_TYPE_TRUE;
  return json;
}

LCH_Json *LCH_JsonFalseCreate() {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }

  json->type = LCH_JSON_TYPE_FALSE;
  return json;
}

LCH_Json *LCH_JsonStringCreate(LCH_Buffer *const str) {
  assert(str != NULL);

  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }

  json->type = LCH_JSON_TYPE_STRING;
  json->str = str;
  return json;
}

LCH_Json *LCH_JsonNumberCreate(const double number) {
  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }

  json->type = LCH_JSON_TYPE_NUMBER;
  json->number = number;
  return json;
}

LCH_Json *LCH_JsonObjectCreate() {
  LCH_Dict *const dict = LCH_DictCreate();
  if (dict == NULL) {
    return NULL;
  }

  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }

  json->type = LCH_JSON_TYPE_OBJECT;
  json->object = dict;
  return json;
}

LCH_Json *LCH_JsonArrayCreate() {
  LCH_List *const list = LCH_ListCreate();
  if (list == NULL) {
    return NULL;
  }

  LCH_Json *const json = (LCH_Json *)calloc(1, sizeof(LCH_Json));
  if (json == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }
  json->type = LCH_JSON_TYPE_ARRAY;
  json->array = list;

  return json;
}

/****************************************************************************/

double LCH_JsonGetNumber(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_NUMBER);

  return json->number;
}

const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  if (!LCH_JsonObjectHasKey(json, key)) {
    LCH_LOG_ERROR(
        "Failed to get value from JSON object: "
        "Entry with key \"%s\" does not exist.",
        LCH_BufferData(key));
    return NULL;
  }

  const LCH_Json *const value = (LCH_Json *)LCH_DictGet(json->object, key);
  assert(value != NULL);

  return value;
}

const LCH_Json *LCH_JsonArrayGet(const LCH_Json *const json,
                                 const size_t index) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);

  const size_t length = LCH_JsonArrayLength(json);
  if (index >= length) {
    LCH_LOG_ERROR(
        "Failed to get value from JSON array: "
        "Index '%zu' is out of bounds (%zu >= %zu)",
        index, index, length);
    return NULL;
  }

  const LCH_Json *const value = (LCH_Json *)LCH_ListGet(json->array, index);
  assert(value != NULL);

  return value;
}

const LCH_Buffer *LCH_JsonStringGetString(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_STRING);
  assert(json->str != NULL);

  return json->str;
}

const LCH_Buffer *LCH_JsonObjectGetString(const LCH_Json *const json,
                                    const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(key != NULL);

  const LCH_Json *const child = LCH_JsonObjectGet(json, key);
  if (child == NULL) {
    return NULL;
  }

  if (!LCH_JsonIsString(child)) {
    const char *const type = LCH_JsonGetTypeAsString(child);
    LCH_LOG_ERROR(
        "Failed to get value from JSON object with key \"%s\": "
        "Expected type string, but found type %s",
        LCH_BufferData(key), type);
    return NULL;
  }

  const LCH_Buffer *const str = LCH_JsonStringGetString(child);
  assert(str != NULL);

  return str;
}

const LCH_Buffer *LCH_JsonArrayGetString(const LCH_Json *const json,
                                         const size_t index) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  if (child == NULL) {
    return NULL;
  }

  if (!LCH_JsonIsString(child)) {
    const char *const type = LCH_JsonGetTypeAsString(child);
    LCH_LOG_ERROR(
        "Failed to get value from JSON array at index %zu: "
        "Expected type string, type %s",
        index, type);
    return NULL;
  }

  const LCH_Buffer *const str = LCH_JsonStringGetString(child);
  assert(str != NULL);

  return str;
}

const LCH_Json *LCH_JsonObjectGetObject(const LCH_Json *const json,
                                        const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(key != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);

  const LCH_Json *child = LCH_JsonObjectGet(json, key);
  if (child == NULL) {
    return NULL;
  }

  if (!LCH_JsonIsObject(child)) {
    const char *const type = LCH_JsonGetTypeAsString(child);
    LCH_LOG_ERROR(
        "Failed to get value from JSON object with key \"%s\": "
        "Expected type object, but found type %s.",
        LCH_BufferData(key), type);
    return NULL;
  }

  return child;
}

const LCH_Json *LCH_JsonArrayGetObject(const LCH_Json *const json,
                                       const size_t index) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);

  const LCH_Json *const child = LCH_JsonArrayGet(json, index);
  if (child == NULL) {
    return NULL;
  }

  if (!LCH_JsonIsObject(child)) {
    const char *const type = LCH_JsonGetTypeAsString(child);
    LCH_LOG_ERROR(
        "Failed to get value from JSON array at index %zu: "
        "Expected type object, but found %s",
        index, type);
    return NULL;
  }

  return child;
}

const LCH_Json *LCH_JsonObjectGetArray(const LCH_Json *const json,
                                       const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(key != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);

  const LCH_Json *child = LCH_JsonObjectGet(json, key);
  if (child == NULL) {
    return NULL;
  }

  if (!LCH_JsonIsArray(child)) {
    const char *const type = LCH_JsonGetTypeAsString(child);
    LCH_LOG_ERROR(
        "Failed to get value from JSON object with key \"%s\": "
        "Expected type array, but found type %s.",
        LCH_BufferData(key), type);
    return NULL;
  }

  return child;
}

/****************************************************************************/

bool LCH_JsonObjectSet(const LCH_Json *const json, const LCH_Buffer *const key,
                       LCH_Json *const value) {
  assert(json != NULL);
  assert(key != NULL);
  assert(value != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  const bool success = LCH_DictSet(json->object, key, value, LCH_JsonDestroy);
  return success;
}

bool LCH_JsonObjectSetString(const LCH_Json *const json, const LCH_Buffer *const key,
                             LCH_Buffer *const str) {
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

bool LCH_JsonObjectSetStringDuplicate(const LCH_Json *const json,
                                      const LCH_Buffer *const key,
                                      const LCH_Buffer *const str) {
  assert(json != NULL);
  assert(key != NULL);
  assert(str != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  LCH_Buffer *const dup = LCH_BufferDuplicate(str);
  if (dup == NULL) {
    return false;
  }

  if (!LCH_JsonObjectSetString(json, key, dup)) {
    LCH_BufferDestroy(dup);
    return false;
  }

  return true;
}

bool LCH_JsonObjectSetNumber(const LCH_Json *const json, const LCH_Buffer *const key,
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

/****************************************************************************/

bool LCH_JsonArrayAppend(const LCH_Json *const json, LCH_Json *const element) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);
  assert(element != NULL);

  const bool success = LCH_ListAppend(json->array, element, LCH_JsonDestroy);
  return success;
}

/****************************************************************************/

LCH_Json *LCH_JsonObjectRemove(const LCH_Json *const json,
                               const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(key != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  LCH_Json *const value = (LCH_Json *)LCH_DictRemove(json->object, key);
  assert(value != NULL);
  return value;
}

LCH_Json *LCH_JsonArrayRemove(const LCH_Json *const json, const size_t index) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);

  LCH_Json *const child = (LCH_Json *)LCH_ListRemove(json->array, index);
  assert(child != NULL);
  return child;
}

LCH_Json *LCH_JsonObjectRemoveObject(const LCH_Json *const json,
                                     const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(key != NULL);

  {
    const LCH_Json *const child = LCH_JsonObjectGet(json, key);
    assert(child != NULL);

    if (!LCH_JsonIsObject(child)) {
      const char *const type = LCH_JsonGetTypeAsString(child);
      LCH_LOG_ERROR(
          "Failed to remove object from object with key \"%s\": "
          "Expected type object, but found %s",
          LCH_BufferData(key), type);
      return NULL;
    }
  }

  LCH_Json *const child = LCH_JsonObjectRemove(json, key);
  assert(child != NULL);
  return child;
}

LCH_Json *LCH_JsonArrayRemoveObject(const LCH_Json *const json,
                                    const size_t index) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);

  {
    const LCH_Json *const child = LCH_JsonArrayGet(json, index);
    assert(child != NULL);

    if (!LCH_JsonIsObject(child)) {
      const char *const type = LCH_JsonGetTypeAsString(child);
      LCH_LOG_ERROR(
          "Failed to remove element at index %zu from array: "
          "Expected type object, but found %s",
          index, type);
      return NULL;
    }
  }

  LCH_Json *const child = LCH_JsonArrayRemove(json, index);
  assert(child != NULL);
  return child;
}

LCH_Json *LCH_JsonObjectRemoveArray(const LCH_Json *const json,
                                    const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(key != NULL);

  {
    const LCH_Json *const child = LCH_JsonObjectGet(json, key);
    assert(child != NULL);

    if (!LCH_JsonIsArray(child)) {
      const char *const type = LCH_JsonGetTypeAsString(child);
      LCH_LOG_ERROR(
          "Failed to remove array from object with key \"%s\": "
          "Expected type array, but found %s",
          LCH_BufferData(key), type);
      return NULL;
    }
  }

  LCH_Json *const child = LCH_JsonObjectRemove(json, key);
  assert(child != NULL);
  return child;
}

LCH_Json *LCH_JsonArrayRemoveArray(const LCH_Json *const json,
                                   const size_t index) {
  assert(json != NULL);

  {
    const LCH_Json *const child = LCH_JsonArrayGet(json, index);
    assert(child != NULL);

    if (!LCH_JsonIsArray(child)) {
      const char *const type = LCH_JsonGetTypeAsString(child);
      LCH_LOG_ERROR(
          "Failed to remove array from array with index %zu: "
          "Expected type array, but found %s",
          index, type);
      return NULL;
    }
  }

  LCH_Json *const child = LCH_JsonArrayRemove(json, index);
  assert(child != NULL);
  return child;
}

/****************************************************************************/

LCH_List *LCH_JsonObjectGetKeys(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  LCH_List *const keys = LCH_DictGetKeys(json->object);
  return keys;
}

bool LCH_JsonObjectHasKey(const LCH_Json *const json, const LCH_Buffer *const key) {
  assert(json != NULL);
  assert(json->object != NULL);
  assert(key != NULL);

  const bool has_key = LCH_DictHasKey(json->object, key);
  return has_key;
}

size_t LCH_JsonObjectLength(const LCH_Json *json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_OBJECT);
  assert(json->object != NULL);

  const size_t length = LCH_DictLength(json->object);
  return length;
}

size_t LCH_JsonArrayLength(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_ARRAY);
  assert(json->array != NULL);

  const size_t length = LCH_ListLength(json->array);
  return length;
}

/****************************************************************************/

LCH_Json *LCH_JsonObjectKeysSetMinus(const LCH_Json *const left,
                                     const LCH_Json *const right) {
  assert(left != NULL);
  assert(LCH_JsonIsObject(left));
  assert(right != NULL);
  assert(LCH_JsonIsObject(right));

  LCH_Json *const result = LCH_JsonObjectCreate();
  if (result == NULL) {
    return NULL;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(left);
  if (keys == NULL) {
    LCH_JsonDestroy(result);
    return NULL;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    if (!LCH_JsonObjectHasKey(right, key)) {
      const LCH_Json *const left_value = LCH_JsonObjectGet(left, key);
      assert(left_value != NULL);

      LCH_Json *const copy = LCH_JsonCopy(left_value);
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
    const LCH_Json *const left, const LCH_Json *const right) {
  assert(left != NULL);
  assert(LCH_JsonIsObject(left));
  assert(right != NULL);
  assert(LCH_JsonIsObject(right));

  LCH_Json *const result = LCH_JsonObjectCreate();
  if (result == NULL) {
    return NULL;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(left);
  if (keys == NULL) {
    LCH_JsonDestroy(result);
    return NULL;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = (char *)LCH_ListGet(keys, i);
    assert(key != NULL);

    if (LCH_JsonObjectHasKey(right, key)) {
      const LCH_Json *const left_value = LCH_JsonObjectGet(left, key);
      assert(left_value != NULL);

      const LCH_Json *const right_value = LCH_JsonObjectGet(right, key);
      assert(right_value != NULL);

      if (!LCH_JsonIsEqual(left_value, right_value)) {
        LCH_Json *const copy = LCH_JsonCopy(left_value);
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

static LCH_Json *JsonNumberCopy(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_NUMBER);

  LCH_Json *copy = LCH_JsonNumberCreate(json->number);
  return copy;
}

static LCH_Json *JsonStringCopy(const LCH_Json *const json) {
  assert(json != NULL);
  assert(json->type == LCH_JSON_TYPE_STRING);
  assert(json->str != NULL);

  LCH_Buffer *const dup = LCH_BufferDuplicate(json->str);
  if (dup == NULL) {
    return NULL;
  }

  LCH_Json *const copy = LCH_JsonStringCreate(dup);
  return copy;
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
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);
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
      abort(); // THIS SHOULD NEVER EVER HAPPEN!
  }
}

/****************************************************************************/

static bool JsonStringEqual(const LCH_Json *const left, const LCH_Json *const right) {
  assert(left != NULL);
  assert(LCH_JsonIsString(left->type));
  assert(left->str != NULL);

  assert(right != NULL);
  assert(LCH_JsonIsString(right->type));
  assert(right->str != NULL);

  const bool is_equal = LCH_BufferEqual(left->str, right->str);
  return is_equal;
}

static bool JsonNumberEqual(const LCH_Json *const left, const LCH_Json *const right) {
  assert(left != NULL);
  assert(LCH_JsonIsNumber(left->type));

  assert(right != NULL);
  assert(LCH_JsonIsNumver(right->type));

  const bool is_equal = left->number == right->number;
  return is_equal;
}

static bool JsonObjectEqual(const LCH_Json *const left, const LCH_Json *const right) {
  assert(left != NULL);
  assert(LCH_BufferIsObject(left->type));
  assert(left->object != NULL);

  assert(right != NULL);
  assert(LCH_BufferIsObject(right->type));
  assert(right->object != NULL);

  const size_t length = LCH_JsonObjectLength(left);
  if (length != LCH_JsonObjectLength(right)) {
    return false;
  }

  LCH_List *const keys = LCH_JsonObjectGetKeys(left);
  assert(length == LCH_ListLength(keys));

  for (size_t i = 0; i < length; i++) {
    const LCH_Buffer *const key = (LCH_Buffer *)LCH_ListGet(keys, i);
    assert(key != NULL);

    if (!LCH_JsonObjectHasKey(right, key)) {
      LCH_ListDestroy(keys);
      return false;
    }

    const LCH_Json *const left_child = LCH_JsonObjectGet(left, key);
    assert(left_child != NULL);

    const LCH_Json *const right_child = LCH_JsonObjectGet(right, key);
    assert(right_child != NULL);

    if (!LCH_JsonIsEqual(left_child, right_child)) {
      LCH_ListDestroy(keys);
      return false;
    }
  }

  LCH_ListDestroy(keys);
  return true;
}

static bool JsonArrayEqual(const LCH_Json *const left, const LCH_Json *const right) {
  assert(left != NULL);
  assert(LCH_JsonIsArray(left->type));
  assert(left->array != NULL);

  assert(right != NULL);
  assert(LCH_JsonIsArray(right->type));
  assert(right->array != NULL);

  const size_t length = LCH_JsonArrayLength(left);
  if (length != LCH_JsonArrayLength(right)) {
    return false;
  }

  for (size_t i = 0; i < length; i++) {
    const LCH_Json *const left_element = (LCH_Json *)LCH_JsonArrayGet(left, i);
    assert(left_element != NULL);

    const LCH_Json *const right_element = (LCH_Json *)LCH_JsonArrayGet(right, i);
    assert(right_element != NULL);

    if (!LCH_JsonIsEqual(left_element, right_element)) {
      return false;
    }
  }

  return true;
}

bool LCH_JsonIsEqual(const LCH_Json *const left, const LCH_Json *right) {
  assert(left != NULL);
  assert(right != NULL);

  if (left->type != right->type) {
    return false;
  }

  switch (left->type) {
    case LCH_JSON_TYPE_NULL:
      // fallthrough

    case LCH_JSON_TYPE_TRUE:
      // fallthrough

    case LCH_JSON_TYPE_FALSE:
      return true;

    case LCH_JSON_TYPE_STRING:
      return JsonStringEqual(left, right);

    case LCH_JSON_TYPE_NUMBER:
      return JsonNumberEqual(left, right);

    case LCH_JSON_TYPE_OBJECT:
      return JsonObjectEqual(left, right);

    case LCH_JSON_TYPE_ARRAY:
      return JsonArrayEqual(left, right);

    default:
      abort(); // THIS SHOULD NEVER EVER HAPPEN!
  }
}

/****************************************************************************/

static void TrimLeadingWhitespace(Parser *const parser) {
  const char *const whitespace = " \r\n\t";
  while (parser->cursor < parser->end) {
    bool is_whitespace = false;
    for (char *ws = whitespace; *ws != '\0'; ws++) {
      if (parser->cursor[0] == *ws) {
        is_whitespace = true;
        break;
      }
    }
    if (is_whitespace) {
      parser->cursor += 1;
    }
    else {
      return;
    }
  }
}

static LCH_Json *Parse(const Parser *parser);

static LCH_Json *ParseNull(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);

  const char *const term = "null";
  const size_t term_len = strlen(term);

  assert((parser->end - parser->cursor) >= term_len);
  assert(strncmp(parser->cursor, term, term_len) == 0);

  LCH_Json *const json = LCH_JsonNullCreate();
  if (json == NULL) {
    return NULL;
  }

  parser->cursor += term_len;
  return json;
}

static LCH_Json *ParseTrue(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);

  const char *const term = "true";
  const size_t term_len = strlen(term);

  assert((parser->end - parser->cursor) >= term_len);
  assert(strncmp(parser->cursor, term, term_len) == 0);

  LCH_Json *const json = LCH_JsonTrueCreate();
  if (json == NULL) {
    return NULL;
  }

  parser->cursor += term_len;
  return json;
}

static LCH_Json *ParseFalse(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);

  const char *const term = "false";
  const size_t term_len = strlen(term);

  assert((parser->end - parser->cursor) >= term_len);
  assert(strncmp(parser->cursor, term, term_len) == 0);

  LCH_Json *const json = LCH_JsonFalseCreate();
  if (json == NULL) {
    return NULL;
  }

  parser->cursor += term_len;
  return json;
}

static LCH_Buffer *BufferParseString(Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);

  assert(parser->cursor[0] == '"');
  parser->cursor++;  // Skip initial double quote

  LCH_Buffer *const str = LCH_BufferCreate();
  if (str == NULL) {
    return NULL;
  }

  while (parser->cursor < parser->end) {
    if (parser->cursor[0] == '\\') {
      parser->cursor += 1;

      if (parser->cursor >= parser) {
        LCH_LOG_ERROR(
            "Failed to parse JSON: Expected control character after backslash, "
            "but reached End-of-Buffer");
        LCH_BufferDestroy(str);
        return NULL;
      }

      switch (parser->cursor[0]) {
        case '"':
          if (!LCH_BufferAppend(str, '"')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case '\\':
          if (!LCH_BufferAppend(str, '\\')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case '/':
          if (!LCH_BufferAppend(str, '/')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case 'b':
          if (!LCH_BufferAppend(str, '\b')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case 'f':
          if (!LCH_BufferAppend(str, '\f')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case 'n':
          if (!LCH_BufferAppend(str, '\n')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case 'r':
          if (!LCH_BufferAppend(str, '\r')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case 't':
          if (!LCH_BufferAppend(str, '\t')) {
            LCH_BufferDestroy(str);
            return NULL;
          }
          break;

        case 'u':
          // TODO: Make sure it function call below does not go past End-of-Buffer
          if (!LCH_BufferUnicodeToUTF8(str, parser->cursor + 1)) {
            LCH_LOG_ERROR(
                "Failed to parse JSON: Illegal unicode control sequence '%.6s'",
                parser->cursor[0]);
            LCH_BufferDestroy(str);
          }
          parser->cursor += 4;
          break;

        default:
          LCH_LOG_ERROR("Failed to parse JSON string: Illegal control character '\\%c'", parser->cursor[0]);
          LCH_BufferDestroy(str);
          return NULL;
      }
      parser->cursor++;
    } else if (!LCH_BufferAppend(str, parser->cursor[0])) {
      LCH_BufferDestroy(str);
      return NULL;
    }
    parser->cursor++;
  }

  if (parser->cursor[0] != '"') {
    LCH_LOG_ERROR(
        "Failed to parse JSON: Syntax error; "
        "expected '\"', but found token '%c'",
        parser->cursor[0]);
    LCH_BufferDestroy(str);
    return NULL;
  }

  return str;
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

static const char *JsonParseObject(const char *str, LCH_Json **json) {
  assert(str != NULL);
  assert(*str == '{');

  LCH_Json *const object = LCH_JsonObjectCreate();
  if (object == NULL) {
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
            "Failed to parse JSON: Syntax error; expected ',', but found '%c'",
            *str);
        LCH_JsonDestroy(object);
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
      LCH_JsonDestroy(object);
      return NULL;
    }
    char *const key = LCH_BufferToString(buffer);

    // Skip whitespace
    str += strspn(str, " \r\n\t");

    // Skip colon
    if (*str != ':') {
      LCH_LOG_ERROR(
          "Failed to parse JSON: Syntax error; expected ':', but found '%c'", *str);
      free(key);
      LCH_JsonDestroy(object);
      return NULL;
    }
    str++;

    // Extract value
    LCH_Json *value;
    str = JsonParse(str, &value);
    if (str == NULL) {
      free(key);
      LCH_JsonDestroy(object);
    }

    if (!LCH_JsonObjectSet(object, key, value)) {
      free(key);
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(object);
      return NULL;
    }

    free(key);

    // Skip whitespace
    str += strspn(str, " \r\n\t");
  }

  if (*str != '}') {
    LCH_LOG_ERROR(
        "Failed to parse JSON string: Syntax error; expected '}', but found '%c'",
        *str);
    LCH_JsonDestroy(object);
    return NULL;
  }

  *json = object;
  return str + 1;
}

static const char *JsonParseArray(const char *str, LCH_Json **json) {
  assert(str != NULL);
  assert(*str == '[');

  LCH_Json *const child = LCH_JsonArrayCreate();
  if (child == NULL) {
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
            "Failed to parse JSON: Syntax error; expected ',', but found '%c'",
            *str);
        LCH_JsonDestroy(child);
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
      LCH_JsonDestroy(child);
      return NULL;
    }

    if (!LCH_JsonArrayAppend(child, value)) {
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(child);
      return NULL;
    }

    // Skip whitespace
    str += strspn(str, " \r\n\t");
  }

  if (*str != ']') {
    LCH_LOG_ERROR(
        "Failed to parse JSON string: Syntax error; expected ']', but found '%c'",
        *str);
    LCH_JsonDestroy(child);
    return NULL;
  }

  *json = child;
  return str + 1;
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

static LCH_Json *Parse(const Parser *const parser) {
  assert(parser != NULL);
  assert(parser->cursor != NULL);
  assert(parser->end != NULL);
  assert(parser->path != NULL);

  TrimLeadingWhitespace(parser);
  const size_t remaining = parser->end - parser->cursor;

  const size_t null_len = strlen("null");
  if ((remaining >= null_len) && LCH_StringEqual(parser->cursor, "null")) {
    return ParseNull(parser);
  }

  const size_t true_len = strlen("true");
  if ((remaining >= true_len) && LCH_StringEqual(parser->cursor, "true")) {
    return ParseTrue(parser);
  }

  const size_t false_len = strlen("false");
  if ((remaining >= false_len) && LCH_StringEqual(parser->cursor, "false")) {
    return ParseFalse(parser);
  }

  if ((remaining >= 1)) {
    if (parser->cursor[0] == '"') {
      return ParseString(parser);
    }

    if (parser->cursor[0] == '{') {
      return ParseObject(parser);
    }

    if (parser->cursor[0] == '[') {
      return ParseArray(parser);
    }
    if (isdigit(parser->cursor[0]) != 0 || parser->cursor[0] == '-') {
      return ParseNumber(parser);
    }
  }

  LCH_LOG_ERROR(
      "Failed to parse JSON: Expected 'null', 'true', 'false', NUMBER, STRING,"
      "OBJECT, ARRAY; but found token '%c'", parser->cursor[0]);
  return NULL;
}

LCH_Json *LCH_JsonParse(const char *const str, const size_t len) {
  assert(str != NULL);

  LCH_List *const path = LCH_ListCreate();
  if (path == NULL) {
    return NULL;
  }

  Parser parser = {
    .cursor = str,
    .end = str + len,
    .path = path,
  };

  LCH_Json *json = Parse(&parser);
  LCH_ListDestroy(path);
  return json;
}

/****************************************************************************/

static bool JsonCompose(const LCH_Json *const json, LCH_Buffer *const buffer);

static bool JsonComposeNull(const LCH_Json *const json,
                            LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_NULL);
  return LCH_BufferPrintFormat(buffer, "null");
}

static bool JsonComposeTrue(const LCH_Json *const json,
                            LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_TRUE);
  return LCH_BufferPrintFormat(buffer, "true");
}

static bool JsonComposeFalse(const LCH_Json *const json,
                             LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_FALSE);
  return LCH_BufferPrintFormat(buffer, "false");
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

static bool JsonComposeNumber(const LCH_Json *const json,
                              LCH_Buffer *const buffer) {
  assert(json != NULL);
  assert(buffer != NULL);
  assert(LCH_JsonGetType(json) == LCH_JSON_TYPE_NUMBER);
  return LCH_BufferPrintFormat(buffer, "%f", json->number);
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

/****************************************************************************/

void LCH_JsonDestroy(void *const self) {
  LCH_Json *const json = (LCH_Json *)self;
  if (json != NULL) {
    free(json->str);
    LCH_ListDestroy(json->array);
    LCH_DictDestroy(json->object);
  }
  free(json);
}
