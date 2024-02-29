#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include <stdlib.h>

#include "json.h"
#include "list.h"

typedef struct LCH_TableInfo LCH_TableInfo;

void LCH_TableInfoDestroy(void *info);

LCH_TableInfo *LCH_TableInfoLoad(const char *identifer,
                                 const LCH_Json *table_info);

const char *LCH_TableInfoGetIdentifier(const LCH_TableInfo *table_info);

LCH_Json *LCH_TableInfoLoadNewState(const LCH_TableInfo *table_info);

LCH_Json *LCH_TableInfoLoadOldState(const LCH_TableInfo *table_info,
                                    const char *work_dir);

bool LCH_TableStoreNewState(const LCH_TableInfo *table_info,
                            const char *work_dir, const LCH_Json *new_state);

bool LCH_TablePatch(const LCH_TableInfo *table_info, const char *field,
                    const char *value, const LCH_Json *inserts,
                    const LCH_Json *deletes, const LCH_Json *updates);

#endif  // _LEECH_TABLE_H
