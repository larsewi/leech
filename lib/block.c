#include "block.h"

#include <assert.h>
#include <limits.h>
#include <time.h>

#include "definitions.h"
#include "files.h"
#include "head.h"
#include "logger.h"
#include "string_lib.h"
#include "utils.h"

LCH_Json *LCH_BlockCreate(const char *const parent_id,
                          LCH_Json *const payload) {
  assert(parent_id != NULL);
  assert(payload != NULL);

  LCH_Json *const block = LCH_JsonObjectCreate();
  if (block == NULL) {
    return NULL;
  }

  {
    const LCH_Buffer key = LCH_BufferStaticFromString("version");
    if (!LCH_JsonObjectSetNumber(block, &key, (double)LCH_BLOCK_VERSION)) {
      LCH_JsonDestroy(block);
      return NULL;
    }
  }

  {
    const time_t timestamp = time(NULL);
    const LCH_Buffer key = LCH_BufferStaticFromString("timestamp");
    if (!LCH_JsonObjectSetNumber(block, &key, (double)timestamp)) {
      LCH_LOG_ERROR("Failed to set timestamp field in block");
      LCH_JsonDestroy(block);
      return NULL;
    }
  }

  LCH_Buffer *const parent = LCH_BufferFromString(parent_id);
  if (parent == NULL) {
    LCH_JsonDestroy(block);
    return NULL;
  }

  {
    const LCH_Buffer key = LCH_BufferStaticFromString("parent");
    if (!LCH_JsonObjectSetString(block, &key, parent)) {
      LCH_LOG_ERROR("Failed to set parent block identifier field in block");
      LCH_BufferDestroy(parent);
      LCH_JsonDestroy(block);
      return NULL;
    }
  }

  const LCH_Buffer key = LCH_BufferStaticFromString("payload");
  if (!LCH_JsonObjectSet(block, &key, payload)) {
    LCH_LOG_ERROR("Failed to set payload field in block");
    LCH_JsonDestroy(block);
    return NULL;
  }

  return block;
}

bool LCH_BlockStore(const LCH_Instance *const instance,
                    const LCH_Json *const block) {
  assert(block != NULL);

  const char *const work_dir = LCH_InstanceGetWorkDirectory(instance);
  const bool pretty_print = LCH_InstanceShouldPrettyPrint(instance);

  LCH_Buffer *const json = LCH_JsonCompose(block, pretty_print);
  if (json == NULL) {
    return false;
  }

  LCH_Buffer *const digest = LCH_BufferCreate();
  if (digest == NULL) {
    LCH_BufferDestroy(json);
    return false;
  }

  const size_t length = LCH_BufferLength(json);
  const unsigned char *const data = (const unsigned char *)LCH_BufferData(json);
  if (!LCH_MessageDigest(data, length, digest)) {
    LCH_BufferDestroy(digest);
    LCH_BufferDestroy(json);
    return false;
  }

  char *const block_id = LCH_BufferToString(digest);
  assert(block_id != NULL);

  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
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

bool LCH_BlockGetVersion(const LCH_Json *const block, size_t *const version) {
  double value;
  const LCH_Buffer key = LCH_BufferStaticFromString("version");
  if (!LCH_JsonObjectGetNumber(block, &key, &value)) {
    return false;
  }

  if (!LCH_DoubleToSize(value, version)) {
    return false;
  }

  return true;
}

LCH_Json *LCH_BlockLoad(const char *const work_dir,
                        const char *const block_id) {
  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, PATH_MAX, 3, work_dir, "blocks", block_id)) {
    return NULL;
  }

  LCH_Json *const block = LCH_JsonParseFile(path);
  if (block == NULL) {
    LCH_LOG_ERROR("Failed to parse block with identifier %.7s", block_id);
    return NULL;
  }
  LCH_LOG_DEBUG("Parsed block with identifier %.7s", block_id);

  size_t version;
  if (!LCH_BlockGetVersion(block, &version)) {
    LCH_JsonDestroy(block);
    return NULL;
  }

  if (version > LCH_PATCH_VERSION) {
    LCH_LOG_ERROR("Unsupported block version %zu", version);
    LCH_JsonDestroy(block);
    return NULL;
  }
  LCH_LOG_DEBUG("Block version number is %zu", version);
  return block;
}

const char *LCH_BlockGetParentId(const LCH_Json *const block) {
  const LCH_Buffer key = LCH_BufferStaticFromString("parent");
  const LCH_Buffer *const parent = LCH_JsonObjectGetString(block, &key);
  if (parent == NULL) {
    LCH_LOG_ERROR("Failed to retrieve parent block identifier");
    return NULL;
  }
  return LCH_BufferData(parent);
}

bool LCH_BlockIsGenisisId(const char *const block_id) {
  return LCH_StringEqual(block_id, LCH_GENISIS_BLOCK_ID);
}

const LCH_Json *LCH_BlockGetPayload(const LCH_Json *const block) {
  const LCH_Buffer key = LCH_BufferStaticFromString("payload");
  const LCH_Json *const payload = LCH_JsonObjectGetArray(block, &key);
  if (payload == NULL) {
    LCH_LOG_ERROR("Failed to get payload from block");
    return NULL;
  }
  return payload;
}

LCH_Json *LCH_BlockRemovePayload(const LCH_Json *const block) {
  const LCH_Buffer key = LCH_BufferStaticFromString("payload");
  LCH_Json *const payload_val = LCH_JsonObjectRemoveArray(block, &key);
  if (payload_val == NULL) {
    LCH_LOG_ERROR("Failed to remove payload from block");
    return NULL;
  }
  return payload_val;
}

bool LCH_BlockGetTimestamp(const LCH_Json *const block,
                           double *const timestamp) {
  const LCH_Buffer key = LCH_BufferStaticFromString("timestamp");
  if (!LCH_JsonObjectGetNumber(block, &key, timestamp)) {
    return false;
  }
  return true;
}

static bool IsValidBlockId(const char *const block_id) {
  assert(block_id != NULL);

  size_t i;
  for (i = 0; block_id[i] != '\0'; i++) {
    if (!(block_id[i] >= '0' && block_id[i] <= '9') &&
        !(block_id[i] >= 'a' && block_id[i] <= 'f')) {
      /* Character is not in range [0-9] or [a-f] */
      return false;
    }
  }

  /* Make sure block ID is 40 characters long */
  const bool correct_length = (i == strlen(LCH_GENISIS_BLOCK_ID));
  return correct_length;
}

char *LCH_BlockIdFromArgument(const char *const work_dir,
                              const char *const argument) {
  assert(work_dir != NULL);
  assert(argument != NULL);

  char path[PATH_MAX];
  if (!LCH_FilePathJoin(path, PATH_MAX, 2, work_dir, "blocks")) {
    /* Error already logged */
    return NULL;
  }

  size_t index, num_matching = 0;

  LCH_List *const blocks = LCH_FileListDirectory(path, true);

  /* Add genesis block to the list */
  char *const genisis_id = LCH_StringDuplicate(LCH_GENISIS_BLOCK_ID);
  if (genisis_id == NULL) {
    LCH_ListDestroy(blocks);
    return NULL;
  }

  if (!LCH_ListAppend(blocks, genisis_id, free)) {
    /* Error already logged */
    free(genisis_id);
    LCH_ListDestroy(blocks);
    return NULL;
  }

  const size_t num_blocks = LCH_ListLength(blocks);

  for (size_t i = 0; i < num_blocks; i++) {
    const char *const filename = (char *)LCH_ListGet(blocks, i);
    if (!IsValidBlockId(filename)) {
      LCH_LOG_WARNING(
          "The file '%s%c%s' does not conform with the block naming convention "
          "and will be ignored",
          path, LCH_PATH_SEP, filename);
    } else if (LCH_StringStartsWith(filename, argument)) {
      index = i;
      num_matching += 1;
    }
  }

  char *block_id = NULL;
  if (num_matching != 1) {
    LCH_LOG_ERROR("%s block identifier '%s': %zu blocks found",
                  (num_matching > 1) ? "Ambiguous" : "Unknown", argument,
                  num_matching);
  } else {
    const char *const filename = (char *)LCH_ListGet(blocks, index);
    block_id = LCH_StringDuplicate(filename);
  }

  LCH_ListDestroy(blocks);
  return block_id;
}
