#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include <stdlib.h>

#include "json.h"
#include "list.h"

typedef struct LCH_TableInfo LCH_TableInfo;

typedef char ***(*LCH_LoadTableCallbackFn)(void *);
typedef void *(*LCH_BeginTxCallbackFn)(void *);
typedef bool (*LCH_EndTxCallbackFn)(void *, int);
typedef bool (*LCH_InsertCallbackFn)(void *, const char *, const char *const *,
                                     const char *const *);
typedef bool (*LCH_DeleteCallbackFn)(void *, const char *, const char *const *,
                                     const char *const *);
typedef bool (*LCH_UpdateCallbackFn)(void *, const char *, const char *const *,
                                     const char *const *);

void LCH_TableInfoDestroy(void *info);

LCH_TableInfo *LCH_TableInfoLoad(const char *identifer,
                                 const LCH_Json *table_info);

const char *LCH_TableInfoGetIdentifier(const LCH_TableInfo *table_info);

const LCH_List *LCH_TableInfoGetPrimaryFields(const LCH_TableInfo *table_info);

const LCH_List *LCH_TableInfoGetSubsidiaryFields(
    const LCH_TableInfo *table_info);

LCH_Json *LCH_TableInfoLoadNewState(const LCH_TableInfo *table_info);

LCH_Json *LCH_TableInfoLoadOldState(const LCH_TableInfo *table_info,
                                    const char *work_dir);

bool LCH_TableStoreNewState(const LCH_TableInfo *table_info,
                            const char *work_dir, const LCH_Json *new_state);

#endif  // _LEECH_TABLE_H
