#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct LCH_Buffer LCH_List;
typedef struct LCH_Buffer LCH_Dict;

LCH_List *LCH_ListCreate();
LCH_Dict *LCH_DictCreate();

size_t LCH_ListLength(const LCH_List *self);
size_t LCH_DictLength(const LCH_Dict *self);

bool LCH_ListAppend(LCH_List *self, void *value, void (*destroy)(void *));

bool LCH_DictSet(LCH_Dict *self, const char *key, void *value,
                 void (*destroy)(void *));

void *LCH_ListGet(const LCH_List *self, size_t index);
void *LCH_DictGet(const LCH_Dict *self, const char *key);

void LCH_ListDestroy(LCH_List *self);
void LCH_DictDestroy(LCH_Dict *self);

LCH_List *LCH_SplitString(const char *str, const char *del);

#endif // _LEECH_UTILS_H
