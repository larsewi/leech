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

  LCH_Buffer *const version = LCH_BufferFromString(PACKAGE_VERSION);
  if (version == NULL) {
    LCH_JsonDestroy(block);
    return NULL;
  }

  if (!LCH_JsonObjectSetString(block, LCH_BufferStaticFromString("version"),
                               version)) {
    LCH_LOG_ERROR("Failed to set version field in block");
    LCH_BufferDestroy(version);
    LCH_JsonDestroy(block);
    return NULL;
  }

  const double timestamp = (double)time(NULL);
  if (!LCH_JsonObjectSetNumber(block, LCH_BufferStaticFromString("timestamp"),
                               timestamp)) {
    LCH_LOG_ERROR("Failed to set timestamp field in block");
    LCH_JsonDestroy(block);
    return NULL;
  }

  LCH_Buffer *const parent = LCH_BufferFromString(parent_id);
  if (parent == NULL) {
    LCH_JsonDestroy(block);
    return NULL;
  }

  if (!LCH_JsonObjectSetString(block, LCH_BufferStaticFromString("parent"),
                               parent)) {
    LCH_LOG_ERROR("Failed to set parent block identifier field in block");
    LCH_BufferDestroy(parent);
    LCH_JsonDestroy(block);
    return NULL;
  }

  if (!LCH_JsonObjectSet(block, LCH_BufferStaticFromString("payload"),
                         payload)) {
    LCH_LOG_ERROR("Failed to set payload field in block");
    LCH_JsonDestroy(block);
    return NULL;
  }

  return block;
}

bool LCH_BlockStore(const char *const work_dir, const LCH_Json *const block) {
  assert(block != NULL);
  assert(work_dir != NULL);

  LCH_Buffer *const json = LCH_JsonCompose(block);
  if (json == NULL) {
    return false;
  }

  LCH_Buffer *const digest = LCH_BufferCreate();
  if (digest == NULL) {
    LCH_BufferDestroy(json);
    return false;
  }

  const size_t length = LCH_BufferLength(json);
  const unsigned char *const data = (unsigned char *)LCH_BufferData(json);
  if (!LCH_MessageDigest(data, length, digest)) {
    LCH_BufferDestroy(digest);
    LCH_BufferDestroy(json);
    return false;
  }

  char *const block_id = LCH_BufferToString(digest);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
    free(block_id);
    LCH_BufferDestroy(json);
    return false;
  }

  if (!LCH_BufferWriteFile(json, path)) {
    free(block_id);
    LCH_BufferDestroy(json);
    return false;
  }
  LCH_BufferDestroy(json);

  if (!LCH_HeadSet("HEAD", work_dir, block_id)) {
    free(block_id);
    return false;
  }
  free(block_id);

  return true;
}

LCH_Json *LCH_BlockLoad(const char *const work_dir,
                        const char *const block_id) {
  char path[PATH_MAX];
  if (!LCH_PathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
    return NULL;
  }

  LCH_Json *const block = LCH_JsonParseFile(path);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to parse block with identifer %.7s", block_id);
    return NULL;
  }

  LCH_LOG_DEBUG("Parsed JSON from block with identifer %.7s", block_id);
  return block;
}

const char *LCH_BlockGetParentBlockIdentifier(const LCH_Json *const block) {
  const LCH_Buffer *const parent =
      LCH_JsonObjectGetString(block, LCH_BufferStaticFromString("parent"));
  if (parent == NULL) {
    LCH_LOG_ERROR("Failed to retrieve parent block identifier");
    return NULL;
  }
  return LCH_BufferData(parent);
}

bool LCH_BlockIsGenisisBlockIdentifier(const char *const block_id) {
  return LCH_StringEqual(block_id, LCH_GENISIS_BLOCK_ID);
}

const LCH_Json *LCH_BlockGetPayload(const LCH_Json *const block) {
  const LCH_Json *const payload =
      LCH_JsonObjectGetArray(block, LCH_BufferStaticFromString("payload"));
  if (payload == NULL) {
    LCH_LOG_ERROR("Failed to get payload from block");
    return NULL;
  }
  return payload;
}

LCH_Json *LCH_BlockRemovePayload(const LCH_Json *const block) {
  LCH_Json *const payload_val =
      LCH_JsonObjectRemoveArray(block, LCH_BufferStaticFromString("payload"));
  if (payload_val == NULL) {
    LCH_LOG_ERROR("Failed to remove payload from block");
    return NULL;
  }
  return payload_val;
}
