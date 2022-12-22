#ifndef _LEECH_BLOCK_H
#define _LEECH_BLOCK_H

#include <openssl/sha.h>
#include <stdbool.h>
#include <time.h>

#define LCH_BLOCK_ID_LENGTH SHA_DIGEST_LENGTH

typedef struct LCH_Block LCH_Block;

char *LCH_BlockGetParentID(const LCH_Block *block);

time_t LCH_BlockGetTimestamp(const LCH_Block *block);

size_t LCH_BlockGetDataLength(const LCH_Block *block);

void *LCH_BlockGetData(LCH_Block *block);

char *LCH_BlockGetBlockID(const LCH_Block *block);

LCH_Block *LCH_BlockCreate(const char *parent_id, const void *data,
                           const size_t data_len);

char *LCH_BlockStore(const char *work_dir, const LCH_Block *block);

LCH_Block *LCH_BlockLoad(const char *work_dir, const char *block_id);

#endif  // _LEECH_BLOCK_H
