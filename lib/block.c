#include "block.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "definitions.h"
#include "leech.h"
#include "utils.h"

struct __attribute__((__packed__)) LCH_Block {
  unsigned char parent_id[SHA_DIGEST_LENGTH];
  uint32_t timestamp;
  uint32_t data_len;
  unsigned char data[0];
};

static char *DigestToString(const unsigned char digest[SHA_DIGEST_LENGTH]) {
  // Two characters per byte, plus terminating null byte
  const int len = (SHA_DIGEST_LENGTH * 2) + 1;

  char *const str = malloc(len);
  if (str == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
    int ret = snprintf(str + (i * 2), len - (i * 2), "%02x", digest[i]);
    assert(ret == 2);
  }

  return str;
}

static bool StringToDigest(const char *str,
                           unsigned char digest[SHA_DIGEST_LENGTH]) {
  for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
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

void *LCH_BlockGetData(LCH_Block *const block) { return block->data; }

char *LCH_BlockGetBlockID(const LCH_Block *const block) {
  unsigned char digest[SHA_DIGEST_LENGTH];

  if (block == NULL) {
    memset(digest, 0, sizeof(digest));
  } else {
    const size_t len = sizeof(LCH_Block) + LCH_BlockGetDataLength(block);
    SHA1((unsigned char *)block, len, digest);
  }

  return DigestToString(digest);
}

LCH_Block *LCH_BlockCreate(const char *const parent_id, const void *const data,
                           const size_t data_len) {
  LCH_Block *block = malloc(sizeof(LCH_Block) + data_len);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for block: %s");
    return NULL;
  }

  assert(StringToDigest(parent_id, block->parent_id));

  const time_t timestamp = time(NULL);
  assert(timestamp <= UINT32_MAX);
  block->timestamp = htonl((uint32_t)timestamp);

  assert(data_len <= UINT32_MAX);
  block->data_len = htonl((uint32_t)data_len);

  memcpy(block->data, data, data_len);

  return block;
}

char *LCH_BlockStore(const char *const work_dir, const LCH_Block *const block) {
  assert(work_dir != NULL);
  assert(block != NULL);

  char *const block_id = LCH_BlockGetBlockID(block);
  if (block_id == NULL) {
    LCH_LOG_ERROR("Failed to compute block ID");
    return NULL;
  }

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "blocks", block_id)) {
    free(block_id);
    return NULL;
  }

  FILE *const file = fopen(path, "wb");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for binary writing: %s", path,
                  strerror(errno));
    free(block_id);
    return NULL;
  }

  if (fwrite(block, sizeof(LCH_Block), 1, file) != 1) {
    LCH_LOG_ERROR("Failed to write block header to file '%s': %s", path,
                  strerror(errno));
    fclose(file);
    free(block_id);
    return NULL;
  }

  const size_t data_len = LCH_BlockGetDataLength(block);
  if (fwrite(block->data, 1, data_len, file) != data_len) {
    LCH_LOG_ERROR("Failed to write block payload to file '%s': %s", path,
                  strerror(errno));
    fclose(file);
    free(block_id);
    return NULL;
  }

  fclose(file);
  return block_id;
}

LCH_Block *LCH_BlockLoad(const char *const work_dir,
                         const char *const block_id) {
  assert(work_dir != NULL);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "blocks", block_id)) {
    return NULL;
  }

  FILE *const file = fopen(path, "rb");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for binary reading: %s", path,
                  strerror(errno));
    return NULL;
  }

  LCH_Block *block = malloc(sizeof(LCH_Block));
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    fclose(file);
    return NULL;
  }

  if (fread(block, sizeof(LCH_Block), 1, file) != 1) {
    LCH_LOG_ERROR("Failed to read block header from file '%s': %s", path,
                  strerror(errno));
    free(block);
    fclose(file);
    return NULL;
  }

  const size_t data_len = LCH_BlockGetDataLength(block);
  {
    void *ptr = realloc(block, sizeof(LCH_Block) + data_len);
    if (ptr == NULL) {
      LCH_LOG_ERROR("Failed to reallocate memory: %s", strerror(errno));
      free(block);
      fclose(file);
      return NULL;
    }
    block = ptr;
  }

  if (fread(block->data, 1, data_len, file) != data_len) {
    LCH_LOG_ERROR("Failed to read block payload from file '%s': %s", path,
                  strerror(errno));
    free(block);
    fclose(file);
    return NULL;
  }

  fclose(file);
  return block;
}

bool LCH_BlockRemove(const char *const work_dir, const char *const block_id) {
  assert(work_dir != NULL);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "blocks", block_id)) {
    return NULL;
  }

  if (unlink(path) != 0) {
    LCH_LOG_ERROR("Failed to delete block at path '%s': %s", path,
                  strerror(errno));
    return false;
  }

  return true;
}
