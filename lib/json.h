#ifndef _LEECH_JSON_H
#define _LEECH_JSON_H

#include <stdlib.h>

#include "leech.h"

typedef struct LCH_Json LCH_Json;

typedef enum {
  LCH_JSON_TYPE_NULL,
  LCH_JSON_TYPE_TRUE,
  LCH_JSON_TYPE_FALSE,
  LCH_JSON_TYPE_STRING,
  LCH_JSON_TYPE_NUMBER,
  LCH_JSON_TYPE_ARRAY,
  LCH_JSON_TYPE_OBJECT,
} LCH_JsonType;

LCH_JsonType LCH_JsonGetType(const LCH_Json *json);

/****************************************************************************/

LCH_Json *LCH_JsonNullCreate();

/****************************************************************************/

LCH_Json *LCH_JsonTrueCreate();

/****************************************************************************/

LCH_Json *LCH_JsonFalseCreate();

/****************************************************************************/

LCH_Json *LCH_JsonStringCreate(char *str);

const char *LCH_JsonStringGet(const LCH_Json *json);

/****************************************************************************/

LCH_Json *LCH_JsonObjectCreate();

LCH_Json *LCH_JsonObjectCreateFromDict(LCH_Dict *dict);

LCH_List *LCH_JsonObjectGetKeys(const LCH_Json *json);

bool LCH_JsonObjectHasKey(const LCH_Json *json, const char *key);

const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const char *key);

bool LCH_JsonObjectSet(LCH_Json *json, const char *key, LCH_Json *value);

bool LCH_JsonObjectSetString(LCH_Json *json, const char *key,
                             const char *value);

bool LCH_JsonObjectSetNumber(LCH_Json *json, const char *key, float number);

size_t LCH_JsonObjectLength(const LCH_Json *json);

LCH_Json *LCH_JsonObjectKeysSetMinus(const LCH_Json *a, const LCH_Json *b);

/**
 * @brief Get a copy of all key-value pairs in the left operand where the key is
 *        not present in the right operand.
 * @param a left operand
 * @param b right operand
 * @return new JSON object or NULL on error
 * @note returned JSON must be free'd with LCH_JsonDestroy
 */
LCH_Json *LCH_JsonObjectKeysSetIntersectAndValuesSetMinus(const LCH_Json *a,
                                                          const LCH_Json *b);

/****************************************************************************/

LCH_Json *LCH_JsonArrayCreate();

LCH_Json *LCH_JsonArrayCreateFromList(LCH_List *list);

const LCH_Json *LCH_JsonArrayGet(const LCH_Json *json, size_t index);

bool LCH_JsonArrayAppend(const LCH_Json *json, LCH_Json *value);

size_t LCH_JsonArrayLength(const LCH_Json *json);

/****************************************************************************/

LCH_Json *LCH_JsonNumberCreate(float number);

float LCH_JsonNumberGet(const LCH_Json *json);

/****************************************************************************/

LCH_Json *LCH_JsonParse(const char *str);

char *LCH_JsonCompose(const LCH_Json *json);

LCH_Json *LCH_JsonCopy(const LCH_Json *json);

bool LCH_JsonEqual(const LCH_Json *a, const LCH_Json *b);

void LCH_JsonDestroy(void *json);

#endif  // _LEECH_JSON_H
