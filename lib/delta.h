#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "dict.h"

typedef struct LCH_Delta LCH_Delta;

LCH_Delta *LCH_DeltaCreate(const LCH_Dict *new_state, const LCH_Dict *old_state);

void LCH_DeltaDestroy(LCH_Delta *delta);

char *LCH_DeltaMarshal(const LCH_Delta *delta);

LCH_Delta *LCH_DeltaUnmarshal(const char *data);

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *delta);

size_t LCH_DeltaGetNumModifications(const LCH_Delta *delta);

#endif // _LEECH_DELTA_H
