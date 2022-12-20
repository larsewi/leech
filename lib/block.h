#ifndef _LEECH_BLOCK_H
#define _LEECH_BLOCK_H

typedef struct LCH_Block LCH_Block;

LCH_Block *LCH_BlockCreate(const LCH_Block *previous);

void LCH_BlockDestroy(LCH_Block *block);

#endif  // _LEECH_BLOCK_H
