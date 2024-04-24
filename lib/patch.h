#ifndef _LEECH_PATCH_H
#define _LEECH_PATCH_H

#include <stdbool.h>

#include "json.h"

bool LCH_PatchGetVersion(const LCH_Json *patch, size_t *version);

LCH_Json *LCH_PatchParse(const char *raw_buffer, size_t raw_length);

LCH_Json *LCH_PatchCreate(const char *lastseen);

bool LCH_PatchAppendBlock(const LCH_Json *patch, LCH_Json *block);

bool LCH_PatchUpdateLastKnown(const LCH_Json *patch, const char *work_dir,
                              const char *identifier);

#endif  // _LEECH_PATCH_H
