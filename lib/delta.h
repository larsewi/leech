#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "buffer.h"
#include "dict.h"

typedef struct LCH_Delta LCH_Delta;

LCH_Delta *LCH_DeltaCreate(const char *table_id, const LCH_Dict *new_state,
                           const LCH_Dict *old_state);

void LCH_DeltaDestroy(LCH_Delta *delta);

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumModifications(const LCH_Delta *delta);

const char *LCH_DeltaGetTableID(const LCH_Delta *delta);

bool LCH_DeltaMarshal(LCH_Buffer *buffer, const LCH_Delta *delta);

const char *LCH_DeltaUnmarshal(LCH_Delta **delta, const char *buffer);

#endif  // _LEECH_DELTA_H
