#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>

typedef struct LCH_List LCH_List;

LCH_List *LCH_ListCreate();

bool LCH_ListLength(LCH_List *list);

bool LCH_ListAppend(LCH_List *list, void *data, void (*destroy)(void *));

void *LCH_ListGet(LCH_List *list, size_t index);

void LCH_ListDestroy(LCH_List *list);

unsigned long LCH_Hash(unsigned char *str);

#endif // _LEECH_UTILS_H
