#include "block.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "leech.h"

struct LCH_Block {
  const unsigned char *parent;
  time_t timestamp;
  size_t length;
  const unsigned *data;
};

LCH_Block *LCH_BlockCreate(const LCH_Block *const previous) {
  LCH_Block *block = calloc(1, sizeof(block));
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for block: %s", strerror(errno));
    return NULL;
  }
  return block;
}

void LCH_BlockDestroy(LCH_Block *block) { free(block); }

void LCH_BlockMarshal(void) {}

void LCH_BlockUnmarshal(void) {}
