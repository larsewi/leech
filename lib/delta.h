#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "buffer.h"
#include "dict.h"
#include "json.h"

LCH_Json *LCH_DeltaCreateV2(const char *const table_id,
                            const LCH_Json *const new_state,
                            const LCH_Json *const old_state);
size_t LCH_DeltaGetNumInsertsV2(const LCH_Json *delta);
size_t LCH_DeltaGetNumDeletesV2(const LCH_Json *delta);
size_t LCH_DeltaGetNumUpdatesV2(const LCH_Json *delta);
const char *LCH_DeltaGetTableIDV2(const LCH_Json *const delta);

/*****************/

typedef struct LCH_Delta LCH_Delta;

LCH_Delta *LCH_DeltaCreate(const LCH_TableDefinition *table_def,
                           const LCH_Dict *new_state,
                           const LCH_Dict *old_state);

void LCH_DeltaDestroy(void *delta);

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumUpdates(const LCH_Delta *delta);

const LCH_Dict *LCH_DeltaGetInsertions(const LCH_Delta *delta);

const LCH_Dict *LCH_DeltaGetDeletions(const LCH_Delta *delta);

const LCH_Dict *LCH_DeltaGetUpdates(const LCH_Delta *delta);

const LCH_TableDefinition *LCH_DeltaGetTable(const LCH_Delta *delta);

size_t LCH_DeltaGetNumMergedOperations(const LCH_Delta *delta);

size_t LCH_DeltaGetNumCanceledOperations(const LCH_Delta *delta);

bool LCH_DeltaMarshal(LCH_Buffer *buffer, const LCH_Delta *delta);

const char *LCH_DeltaUnmarshal(LCH_Delta **delta, const LCH_Instance *instance,
                               const char *buffer);

bool LCH_DeltaCompress(LCH_Delta *child, const LCH_Delta *parent);

#endif  // _LEECH_DELTA_H
