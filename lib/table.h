#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include <stdbool.h>
#include <stdlib.h>

#include "json.h"
#include "list.h"

typedef struct LCH_TableInfo LCH_TableInfo;

void LCH_TableInfoDestroy(void *info);

LCH_TableInfo *LCH_TableInfoLoad(const char *identifer,
                                 const LCH_Json *table_info);

const char *LCH_TableInfoGetIdentifier(const LCH_TableInfo *table_info);

/**
 * @brief Whether or not the "merge_blocks" field is set for this table
 * @param table_info The table definition
 * @return True if table should be merged, otherwise false
 */
bool LCH_TableInfoShouldMergeTable(const LCH_TableInfo *table_info);

LCH_Json *LCH_TableInfoLoadNewState(const LCH_TableInfo *table_info);

LCH_Json *LCH_TableInfoLoadOldState(const LCH_TableInfo *table_info,
                                    const char *work_dir);

bool LCH_TableStoreNewState(const LCH_TableInfo *table_info,
                            const char *work_dir, bool pretty_print,
                            const LCH_Json *new_state);

bool LCH_TablePatch(const LCH_TableInfo *table_info, const char *type,
                    const char *field, const char *value,
                    const LCH_Json *inserts, const LCH_Json *deletes,
                    const LCH_Json *updates);

const LCH_List *LCH_TableInfoGetPrimaryFields(const LCH_TableInfo *table_info);

const LCH_List *LCH_TableInfoGetSubsidiaryFields(
    const LCH_TableInfo *table_info);

const LCH_List *LCH_TableInfoGetAllFields(const LCH_TableInfo *table_info);

#endif  // _LEECH_TABLE_H
