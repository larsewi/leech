#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "json.h"

LCH_Json *LCH_DeltaCreate(const char *table_id, const char *delta,
                          const LCH_Json *new_state, const LCH_Json *old_state);
const char *LCH_DeltaGetTableId(const LCH_Json *delta);
const char *LCH_DeltaGetType(const LCH_Json *delta);
const LCH_Json *LCH_DeltaGetInserts(const LCH_Json *delta);
const LCH_Json *LCH_DeltaGetDeletes(const LCH_Json *delta);
const LCH_Json *LCH_DeltaGetUpdates(const LCH_Json *delta);
bool LCH_DeltaGetNumOperations(const LCH_Json *delta, size_t *num_inserts,
                               size_t *num_deletes, size_t *num_updates);
bool LCH_DeltaMerge(const LCH_Json *parent, LCH_Json *child);

#endif  // _LEECH_DELTA_H
