#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include <stdbool.h>

#include "list.h"

typedef struct LCH_Table LCH_Table;

typedef struct LCH_TableCreateInfo {
    char *readLocator;
    LCH_List *(*readCallback)(const char *);
    char *writeLocator;
    bool (*writeCallback)(const char *, const LCH_List *);
} LCH_TableCreateInfo;

LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo);

void LCH_TableDestroy(LCH_Table *table);

#endif  // _LEECH_TABLE_H
