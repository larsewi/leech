#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include "dict.h"
#include "leech.h"

const char *LCH_TableGetIdentifier(const LCH_Table *table);

LCH_Dict *LCH_TableLoadNewState(const LCH_Table *table);

LCH_Dict *LCH_TableLoadOldState(const LCH_Table *table, const char *work_dir);

#endif  // _LEECH_TABLE_H
