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

size_t LCH_ArrayLength(LCH_Array *array);

bool LCH_ArrayAppendArray(LCH_Array *array, LCH_Array *data);
bool LCH_ArrayAppendObject(LCH_Array *array, LCH_Object *data);
bool LCH_ArrayAppendString(LCH_Array *array, char *data);
bool LCH_ArrayAppendNumber(LCH_Array *array, long data);
bool LCH_ArrayAppendBoolean(LCH_Array *array, bool data);

bool LCH_ArrayGetArray(LCH_Array *array, size_t index, LCH_Array **data);
bool LCH_ArrayGetObject(LCH_Array *array, size_t index, LCH_Object **data);
bool LCH_ArrayGetString(LCH_Array *array, size_t index, char **data);
bool LCH_ArrayGetNumber(LCH_Array *array, size_t index, long *data);
bool LCH_ArrayGetBoolean(LCH_Array *array, size_t index, bool *data);

void LCH_ArrayDestroy(LCH_Array *array);
void LCH_ObjectDestroy(LCH_Object *object);

LCH_Array *LCH_SplitString(const char *str, const char *del);

unsigned long LCH_Hash(char *str);

#endif // _LEECH_UTILS_H
