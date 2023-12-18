#ifndef _LEECH_BLOCK_H
#define _LEECH_BLOCK_H

#include <stdbool.h>
#include <time.h>

#include "buffer.h"
#include "json.h"

LCH_Json *LCH_BlockCreate(const char *work_dir, LCH_Json *const payload);
bool LCH_BlockStore(const LCH_Json *block, const char *work_dir);

#endif  // _LEECH_BLOCK_H
