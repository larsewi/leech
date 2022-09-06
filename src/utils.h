#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>
#include <stdlib.h>

typedef enum LCH_Type {
  LCH_ARRAY,
  LCH_OBJECT,
  LCH_STRING,
  LCH_NUMBER,
  LCH_BOOLEAN,
} LCH_Type;

typedef struct LCH_Array LCH_Array;
typedef struct LCH_Object LCH_Object;

LCH_Array *LCH_ArrayCreate();

size_t LCH_ArrayLength(const LCH_Array *array);

bool LCH_ArrayAppendArray(LCH_Array *array, const LCH_Array *data);
bool LCH_ArrayAppendObject(LCH_Array *array, const LCH_Object *data);
bool LCH_ArrayAppendString(LCH_Array *array, const char *data);
bool LCH_ArrayAppendNumber(LCH_Array *array, long data);
bool LCH_ArrayAppendBoolean(LCH_Array *array, bool data);

LCH_Array *LCH_ArrayGetArray(const LCH_Array *array, size_t index);
LCH_Object *LCH_ArrayGetObject(const LCH_Array *array, size_t index);
char *LCH_ArrayGetString(const LCH_Array *array, size_t index);
long LCH_ArrayGetNumber(const LCH_Array *array, size_t index);
bool LCH_ArrayGetBoolean(const LCH_Array *array, size_t index);

void LCH_ArrayDestroy(LCH_Array *array);
void LCH_ObjectDestroy(LCH_Object *object);

LCH_Array *LCH_SplitString(const char *str, const char *del);

unsigned long LCH_Hash(char *str);

#endif // _LEECH_UTILS_H
