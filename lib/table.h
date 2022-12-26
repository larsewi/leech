#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include "dict.h"
#include "leech.h"

char *LCH_TableGetIdentifier(const LCH_Table *self);

LCH_Dict *LCH_TableLoadNewData(const LCH_Table *table);

LCH_Dict *LCH_TableLoadOldData(const LCH_Table *table, const char *work_dir);

#endif  // _LEECH_TABLE_H
