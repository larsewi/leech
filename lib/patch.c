#include "patch.h"

#include <assert.h>
#include <time.h>

#include "head.h"

LCH_Json *LCH_PatchCreate(const char *const lastknown) {
  assert(lastknown != NULL);

  LCH_Json *const patch = LCH_JsonObjectCreate();
  if (patch == NULL) {
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(patch, "version", PACKAGE_VERSION)) {
    LCH_LOG_ERROR("Failed to set version field in patch");
    LCH_JsonDestroy(patch);
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(patch, "lastknown", lastknown)) {
    LCH_LOG_ERROR("Failed to set lastknown field in patch");
    LCH_JsonDestroy(patch);
    return NULL;
  }

  const double timestamp = (double)time(NULL);
  if (!LCH_JsonObjectSetNumber(patch, "timestamp", timestamp)) {
    LCH_LOG_ERROR("Failed to set timestamp field in patch");
    LCH_JsonDestroy(patch);
    return NULL;
  }

  LCH_Json *const blocks = LCH_JsonArrayCreate();
  if (blocks == NULL) {
    LCH_LOG_ERROR("Failed to create array to hold blocks in patch");
    LCH_JsonDestroy(patch);
    return NULL;
  }

  if (!LCH_JsonObjectSet(patch, "blocks", blocks)) {
    LCH_LOG_ERROR("Failed to set blocks field in patch");
    LCH_JsonDestroy(blocks);
    LCH_JsonDestroy(patch);
    return NULL;
  }

  return patch;
}

bool LCH_PatchAppendBlock(const LCH_Json *const patch, LCH_Json *const block) {
  assert(patch != NULL);
  assert(block != NULL);

  const LCH_Json *const blocks = LCH_JsonObjectGetArray(patch, "blocks");
  if (blocks == NULL) {
    return false;
  }

  if (!LCH_JsonArrayAppend(blocks, block)) {
    return false;
  }

  return true;
}

bool LCH_PatchUpdateLastseen(const LCH_Json *const patch,
                             const char *const work_dir,
                             const char *const identifier) {
  assert(patch != NULL);
  assert(identifier != NULL);

  const char *const lastknown = LCH_JsonObjectGetString(patch, "lastknown");
  if (lastknown == NULL) {
    return false;
  }

  if (!LCH_HeadSet(identifier, work_dir, lastknown)) {
    return false;
  }

  return true;
}
