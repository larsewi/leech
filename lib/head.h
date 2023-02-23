#ifndef _LEECH_HEAD_H
#define _LEECH_HEAD_H

#include <stdbool.h>

#define LCH_GENISIS_BLOCK_PARENT "0000000000000000000000000000000000000000"

char *LCH_HeadGet(const char *const name, const char *work_dir);

bool LCH_HeadSet(const char *const name, const char *workdir, const char *block_id);

#endif  // _LEECH_HEAD_H
