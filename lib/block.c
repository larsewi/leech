#include "block.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "leech.h"
#include "utils.h"

struct __attribute__((__packed__)) LCH_Block {
  unsigned char parent_id[LCH_BLOCK_ID_LENGTH];
  uint32_t timestamp;
  uint32_t data_len;
  unsigned char data[0];
};

static char *DigestToString(const unsigned char digest[LCH_BLOCK_ID_LENGTH]) {
  // Two characters per byte, plus terminating null byte
  const int len = (sizeof(digest) * 2) + 1;

  char *const str = malloc(len);
  if (str == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for block ID: %s",
                  strerror(errno));
    return NULL;
  }

  for (int i = 0; i < sizeof(digest); i++) {
    int ret = snprintf(str + (i * 2), len - (i * 2), "%02x", digest[i]);
    assert(ret == 2);
  }

  return str;
}

static bool StringToDigest(const char *str,
                           unsigned char digest[LCH_BLOCK_ID_LENGTH]) {
  for (int i = 0; i < sizeof(digest); i++) {
    if (sscanf(str, "%2hhx", &digest[i]) != 1) {
      return false;
    }
    str += 2;
  }

  return true;
}

char *LCH_BlockGetParentID(const LCH_Block *const block) {
  return DigestToString(block->parent_id);
}

time_t LCH_BlockGetTimestamp(const LCH_Block *const block) {
  return (time_t)ntohl(block->timestamp);
}

size_t LCH_BlockGetDataLength(const LCH_Block *const block) {
  return (size_t)ntohl(block->data_len);
}

char *LCH_BlockGetBlockID(const LCH_Block *const block) {
  const size_t len = sizeof(LCH_Block) + LCH_BlockGetDataLength(block);
  unsigned char digest[LCH_BLOCK_ID_LENGTH];
  SHA1(block, len, digest);
  return DigestToString(digest);
}

LCH_Block *LCH_BlockCreate(const LCH_Block *const parent, const void *const data,
                     const size_t data_len) {
  assert(parent != NULL);

  LCH_Block *block = malloc(sizeof(LCH_Block) + data_len);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for block: %s");
    return NULL;
  }

  char *parent_id = LCH_BlockGetBlockID(parent);
  if (parent_id == NULL) {
    LCH_LOG_ERROR("Failed to compute parent block ID");
    return NULL;
  }
  assert(StringToDigest(parent_id, block->parent_id));
  free(parent_id);

  const time_t timestamp = time(NULL);
  assert(timestamp <= UINT32_MAX);
  block->timestamp = htons((uint32_t)timestamp);

  assert(data_len <= UINT32_MAX);
  block->data_len = htons((uint32_t)data_len);

  memcpy(block->data, data, data_len);

  return block;
}

bool LCH_BlockStore(const char *const workdir, LCH_Block *block) {

}

LCH_Block *LCH_BlockLoad(const char *const work_dir, const char *const block_id) {
  char path[PATH_MAX];
}


void LCH_BlockDestroy(LCH_Block *const block) { free(block); }
