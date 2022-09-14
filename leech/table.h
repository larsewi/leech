#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include <stdbool.h>

typedef struct LCH_Table LCH_Table;

typedef struct LCH_TableCreateInfo {
  char *readLocator;
  bool (*readCallback)();
  char *writeLocator;
  bool (*writeCallback)();
} LCH_TableCreateInfo;

LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo);

void LCH_TableDestroy(LCH_Table *table);

#endif // _LEECH_TABLE_H
