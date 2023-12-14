#ifndef _LEECH_TABLE_H
#define _LEECH_TABLE_H

#include "delta.h"
#include "dict.h"
#include "leech.h"

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
