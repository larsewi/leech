#ifndef _LCH_BLOCK_H
#define _LCH_BLOCK_H

#include <time.h>

typedef struct LCH_Block_t {
  unsigned char *prev_hash;
  unsigned char *curr_hash;
  unsigned long block_number;
  time_t timestamp;
} LCH_Block;

#endif // _LCH_BLOCK_H
