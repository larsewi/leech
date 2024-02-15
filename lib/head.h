#ifndef _LEECH_HEAD_H
#define _LEECH_HEAD_H

#include <stdbool.h>

char *LCH_HeadGet(const char *work_dir);

bool LCH_HeadSet(const char *work_dir, const char *block_id);

#endif  // _LEECH_HEAD_H
