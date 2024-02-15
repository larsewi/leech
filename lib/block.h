#ifndef _LEECH_BLOCK_H
#define _LEECH_BLOCK_H

#include <stdbool.h>
#include <time.h>

#include "buffer.h"
#include "json.h"

typedef LCH_Json LCH_Block;

void LCH_BlockDestroy(void *block);
LCH_Block *LCH_BlockCreate(const char *parent_id, LCH_Json *const payload);
bool LCH_BlockStore(const char *const work_dir, const LCH_Block *block);
LCH_Block *LCH_BlockLoad(const char *work_dir, const char *block_id);
const char *LCH_BlockGetParentBlockIdentifier(const LCH_Block *block);
bool LCH_BlockIsGenisisBlockIdentifier(const char *block_id);

#endif  // _LEECH_BLOCK_H
