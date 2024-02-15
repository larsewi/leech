#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "buffer.h"
#include "dict.h"
#include "json.h"

typedef LCH_Json LCH_Delta;

void LCH_DeltaDestroy(void *delta);
LCH_Delta *LCH_DeltaCreate(const char *const table_id,
                           const LCH_Json *const new_state,
                           const LCH_Json *const old_state);
size_t LCH_DeltaGetNumInserts(const LCH_Delta *delta);
size_t LCH_DeltaGetNumDeletes(const LCH_Delta *delta);
size_t LCH_DeltaGetNumUpdates(const LCH_Delta *delta);
const char *LCH_DeltaGetTableID(const LCH_Delta *const delta);

#endif  // _LEECH_DELTA_H
