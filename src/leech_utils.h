#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>

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

bool ArrayGetArray(LCH_Array *array, size_t index, LCH_Array **data);
bool ArrayGetObject(LCH_Array *array, size_t index, LCH_Object **data);
bool ArrayGetString(LCH_Array *array, size_t index, char **data);
bool ArrayGetNumber(LCH_Array *array, size_t index, long *data);
bool ArrayGetBoolean(LCH_Array *array, size_t index, bool *data);

void LCH_ArrayDestroy(LCH_Array *array);

unsigned long LCH_Hash(unsigned char *str);

#endif // _LEECH_UTILS_H
