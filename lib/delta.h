#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "json.h"

LCH_Json *LCH_DeltaCreate(const char *table_id, const char *delta,
                          const LCH_Json *new_state, const LCH_Json *old_state);
size_t LCH_DeltaGetNumInserts(const LCH_Json *delta);
size_t LCH_DeltaGetNumDeletes(const LCH_Json *delta);
size_t LCH_DeltaGetNumUpdates(const LCH_Json *delta);
const char *LCH_DeltaGetTableID(const LCH_Json *delta);
bool LCH_DeltaMerge(const LCH_Json *parent, LCH_Json *child);

#endif  // _LEECH_DELTA_H
