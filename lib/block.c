#include "block.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include "definitions.h"
#include "head.h"
#include "logger.h"
#include "utils.h"

LCH_Json *LCH_BlockCreate(const char *const parent_id,
                          LCH_Json *const payload) {
  assert(parent_id != NULL);
  assert(payload != NULL);

  LCH_Json *const block = LCH_JsonObjectCreate();
  if (block == NULL) {
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(block, "version", PACKAGE_VERSION)) {
    LCH_LOG_ERROR("Failed to set version field in block");
    LCH_JsonDestroy(block);
    return NULL;
  }

  const double timestamp = (double)time(NULL);
  if (!LCH_JsonObjectSetNumber(block, "timestamp", timestamp)) {
    LCH_LOG_ERROR("Failed to set timestamp field in block");
    LCH_JsonDestroy(block);
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(block, "parent", parent_id)) {
    LCH_LOG_ERROR("Failed to set parent block identifier field in block");
    LCH_JsonDestroy(block);
    return NULL;
  }

  if (!LCH_JsonObjectSet(block, "payload", payload)) {
    LCH_LOG_ERROR("Failed to set payload field in block");
    LCH_JsonDestroy(block);
    return NULL;
  }

  return block;
}

bool LCH_BlockStore(const char *const work_dir, const LCH_Json *const block) {
  assert(block != NULL);
  assert(work_dir != NULL);

  char *const json = LCH_JsonCompose(block);
  if (json == NULL) {
    return false;
  }

  LCH_Buffer *const digest = LCH_BufferCreate();
  if (digest == NULL) {
    free(json);
    return false;
  }

  if (!LCH_MessageDigest((unsigned char *)json, strlen(json), digest)) {
    LCH_BufferDestroy(digest);
    free(json);
    return false;
  }

  char *const block_id = LCH_BufferToString(digest);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
    free(block_id);
    free(json);
    return false;
  }

  if (!LCH_FileWrite(path, json)) {
    free(json);
    return false;
  }

  if (!LCH_HeadSet("HEAD", work_dir, block_id)) {
    free(block_id);
    free(json);
  }

  free(block_id);
  free(json);
  return true;
}

LCH_Json *LCH_BlockLoad(const char *const work_dir,
                        const char *const block_id) {
  char path[PATH_MAX];
  if (!LCH_PathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
    return NULL;
  }

  size_t num_bytes;
  char *const raw = LCH_FileRead(path, &num_bytes);
  if (raw == NULL) {
    LCH_LOG_ERROR("Failed to read block %.7s", block_id);
    return NULL;
  }
  LCH_LOG_DEBUG("Read JSON from block with identifer %.7s", block_id);

  LCH_Json *const block = LCH_JsonParse(raw);
  free(raw);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to parse block with identifer %.7s", block_id);
    return NULL;
  }
  LCH_LOG_DEBUG("Parsed JSON from block with identifer %.7s", block_id);

  return block;
}

const char *LCH_BlockGetParentBlockIdentifier(const LCH_Json *const block) {
  assert(block != NULL);

  const char *const parent_id = LCH_JsonObjectGetString(block, "parent");
  if (parent_id == NULL) {
    LCH_LOG_ERROR("Failed to retrieve parent block identifier");
    return NULL;
  }
  return parent_id;
}

bool LCH_BlockIsGenisisBlockIdentifier(const char *const block_id) {
  assert(block_id != NULL);
  return LCH_StringEqual(block_id, LCH_GENISIS_BLOCK_ID);
}

const LCH_Json *LCH_BlockGetPayload(const LCH_Json *const block) {
  assert(block != NULL);

  const LCH_Json *const payload = LCH_JsonObjectGetArray(block, "payload");
  if (payload == NULL) {
    LCH_LOG_ERROR("Failed to get payload from block");
    return NULL;
  }
  return payload;
}

LCH_Json *LCH_BlockRemovePayload(const LCH_Json *const block) {
  assert(block != NULL);

  LCH_Json *const payload = LCH_JsonObjectRemoveArray(block, "payload");
  if (payload == NULL) {
    LCH_LOG_ERROR("Failed to remove payload from block");
    return NULL;
  }
  return payload;
}
