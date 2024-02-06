#include "block.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "definitions.h"
#include "head.h"
#include "leech.h"
#include "utils.h"

LCH_Json *LCH_BlockCreate(const char *const work_dir, LCH_Json *const payload) {
  LCH_Json *const block = LCH_JsonObjectCreate();
  if (block == NULL) {
    return NULL;
  }

  const double timestamp = (double)time(NULL);
  if (!LCH_JsonObjectSetNumber(block, "timestamp", timestamp)) {
    LCH_JsonDestroy(block);
    return NULL;
  }

  char *const head = LCH_HeadGet("HEAD", work_dir);
  if (head == NULL) {
    LCH_LOG_ERROR("Failed to get head.");
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(block, "parent", head)) {
    free(head);
    LCH_JsonDestroy(block);
    return NULL;
  }
  free(head);

  if (!LCH_JsonObjectSet(block, "payload", payload)) {
    LCH_JsonDestroy(block);
    return NULL;
  }

  return block;
}

bool LCH_BlockStore(const LCH_Json *const block, const char *const work_dir) {
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

  if (!LCH_HeadSet("head", work_dir, block_id)) {
    free(block_id);
    free(json);
  }

  free(block_id);
  free(json);
  return true;
}
