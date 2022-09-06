#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>
#include <stdlib.h>


typedef struct LCH_Buffer LCH_List;
typedef struct LCH_Buffer LCH_Dict;

LCH_List *LCH_ListCreate();
LCH_Dict *LCH_DictCreate();

size_t LCH_ListLength(const LCH_List *list);
size_t LCH_DictLength(const LCH_Dict *dict);

bool LCH_ListAppend(LCH_List *list, void *data);

void *LCH_ListGet(const LCH_List *list, size_t index);

void LCH_ListDestroy(LCH_List *list);
void LCH_DictDestroy(LCH_Dict *dict);

LCH_List *LCH_SplitString(const char *str, const char *del);

unsigned long LCH_Hash(char *str);

#endif // _LEECH_UTILS_H
