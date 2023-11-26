#ifndef _LEECH_JSON_H
#define _LEECH_JSON_H

#include <stdlib.h>

typedef struct LCH_Json LCH_Json;

typedef enum {
  LCH_JSON_TYPE_NULL,
  LCH_JSON_TYPE_TRUE,
  LCH_JSON_TYPE_FALSE,
  LCH_JSON_TYPE_STRING,
  LCH_JSON_TYPE_NUMBER,
  LCH_JSON_TYPE_LIST,
  LCH_JSON_TYPE_OBJECT,
} LCH_JsonType;

LCH_JsonType LCH_JsonGetType(const LCH_Json *json);

/****************************************************************************/

const char *LCH_JsonStringGet(const LCH_Json *json);

/****************************************************************************/

float LCH_JsonNumberGet(const LCH_Json *json);

/****************************************************************************/

const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const char *key);

size_t LCH_JsonObjectLength(const LCH_Json *json);

/****************************************************************************/

const LCH_Json *LCH_JsonListGet(const LCH_Json *json, size_t index);

size_t LCH_JsonObjectLength(const LCH_Json *json);

/****************************************************************************/

LCH_Json *LCH_JsonParse(const char *str);

void LCH_JsonDestroy(LCH_Json *json);

#endif  // _LEECH_JSON_H
