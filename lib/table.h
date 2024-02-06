#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include "delta.h"
#include "dict.h"
#include "leech.h"

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

LCH_TableInfo *LCH_TableInfoLoad(const char *identifer,
                                 const LCH_Json *definition);

const char *LCH_TableDefinitionGetIdentifier(
    const LCH_TableDefinition *definition);

const char *LCH_TableDefinitionGetPrimaryFields(
    const LCH_TableDefinition *defintion);

const char *LCH_TableDefinitionGetSubsidiaryFields(
    const LCH_TableDefinition *definition);

LCH_Json *LCH_TableDefinitionLoadNewState(
    const LCH_TableDefinition *definition);

LCH_Json *LCH_TableDefinitionLoadOldState(const LCH_TableDefinition *defintion,
                                          const char *work_dir);

bool LCH_TableStoreNewState(const LCH_TableDefinition *definition,
                            const char *work_dir, const LCH_Json *new_state);

#endif  // _LEECH_TABLE_H
