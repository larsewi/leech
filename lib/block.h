#ifndef _LEECH_BLOCK_H
#define _LEECH_BLOCK_H

#include <stdbool.h>

#include "instance.h"
#include "json.h"

LCH_Json *LCH_BlockCreate(const char *parent_id, LCH_Json *const payload);
bool LCH_BlockStore(const LCH_Instance *const instance, const LCH_Json *block);
LCH_Json *LCH_BlockLoad(const char *work_dir, const char *block_id);
bool LCH_BlockGetVersion(const LCH_Json *patch, size_t *version);
const char *LCH_BlockGetParentId(const LCH_Json *block);
bool LCH_BlockIsGenisisId(const char *block_id);
const LCH_Json *LCH_BlockGetPayload(const LCH_Json *block);
LCH_Json *LCH_BlockRemovePayload(const LCH_Json *block);
bool LCH_BlockGetTimestamp(const LCH_Json *block, double *timestamp);

#endif  // _LEECH_BLOCK_H
