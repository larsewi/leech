#include "block.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "leech.h"
#include "utils.h"

struct LCH_Block {
  const unsigned char *parent;
  time_t timestamp;
  size_t length;
  const void *data;
};

LCH_Block *LCH_BlockCreate(const unsigned char *const parent,
                           const void *const data, const size_t length) {
  LCH_Block *block = calloc(1, sizeof(block));
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for block: %s", strerror(errno));
    return NULL;
  }
  block->parent = parent;
  block->timestamp = time(NULL);
  block->length = length;
  block->data = data;

  return block;
}

void LCH_BlockDestroy(LCH_Block *const block) { free(block); }

void *LCH_BlockMarshal(const LCH_Block *const block, size_t *const size) {
  void *const buffer =
      malloc(LCH_SHA1_LENGTH + (2 * sizeof(uint32_t)) + block->length);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for block marshaling: %s",
                  strerror(errno));
    return NULL;
  }

  memcpy(buffer, block->parent, LCH_SHA1_LENGTH);

  const uint32_t time = htonl((uint32_t)block->timestamp);
  memcpy(buffer + LCH_SHA1_LENGTH, &time, sizeof(uint32_t));

  const uint32_t length = htonl((uint32_t)block->length);
  memcpy(buffer + LCH_SHA1_LENGTH + sizeof(uint32_t), &length,
         sizeof(uint32_t));

  memcpy(buffer + LCH_SHA1_LENGTH + (2 * sizeof(uint32_t)), block->data,
         block->length);

  return buffer;
}

void LCH_BlockUnmarshal(void) {}
