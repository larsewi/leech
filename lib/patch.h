#ifndef _LEECH_PATCH_H
#define _LEECH_PATCH_H

#include <stdbool.h>

#include "json.h"

LCH_Json *LCH_PatchCreate(const char *lastseen);

bool LCH_PatchAppendBlock(const LCH_Json *patch, LCH_Json *block);

bool LCH_PatchUpdateLastKnown(const LCH_Json *patch, const char *work_dir,
                              const char *identifier);

#endif  // _LEECH_PATCH_H
