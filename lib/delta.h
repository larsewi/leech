#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "buffer.h"
#include "dict.h"

typedef struct LCH_Delta LCH_Delta;

LCH_Delta *LCH_DeltaCreate(const LCH_Table *table, const LCH_Dict *new_state,
                           const LCH_Dict *old_state);

void LCH_DeltaDestroy(LCH_Delta *delta);

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumUpdates(const LCH_Delta *delta);

const LCH_Dict *LCH_DeltaGetInsertions(const LCH_Delta *delta);

const LCH_Dict *LCH_DeltaGetDeletions(const LCH_Delta *delta);

const LCH_Dict *LCH_DeltaGetUpdates(const LCH_Delta *delta);

const LCH_Table *LCH_DeltaGetTable(const LCH_Delta *delta);

size_t LCH_DeltaGetNumMergedOperations(const LCH_Delta *delta);

size_t LCH_DeltaGetNumCanceledOperations(const LCH_Delta *delta);

bool LCH_DeltaMarshal(LCH_Buffer *buffer, const LCH_Delta *delta);

const char *LCH_DeltaUnmarshal(LCH_Delta **delta, const LCH_Instance *instance,
                               const char *buffer);

bool LCH_DeltaCompress(LCH_Delta *child, const LCH_Delta *parent);

#endif  // _LEECH_DELTA_H
