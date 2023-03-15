#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include "delta.h"
#include "dict.h"
#include "leech.h"

const char *LCH_TableGetIdentifier(const LCH_Table *table);

const char *LCH_TableGetPrimaryFields(const LCH_Table *table);

const char *LCH_TableGetSubsidiaryFields(const LCH_Table *table);

LCH_Dict *LCH_TableLoadNewState(const LCH_Table *table);

LCH_Dict *LCH_TableLoadOldState(const LCH_Table *table, const char *work_dir);

bool LCH_TableStoreNewState(const LCH_Table *self, const char *work_dir,
                            const LCH_Dict *new_state);

bool LCH_TablePatch(const LCH_Table *table, const LCH_Delta *patch,
                    const char *uid_field, const char *uid_value);

#endif  // _LEECH_TABLE_H
