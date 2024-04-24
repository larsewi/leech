#include "patch.h"

#include <assert.h>
#include <time.h>

#include "definitions.h"
#include "files.h"
#include "head.h"
#include "logger.h"
#include "utils.h"

bool LCH_PatchGetVersion(const LCH_Json *const patch, size_t *const version) {
  double value;
  const LCH_Buffer *const key = LCH_BufferStaticFromString("version");
  if (!LCH_JsonObjectGetNumber(patch, key, &value)) {
    return false;
  }

  if (!LCH_DoubleToSize(value, version)) {
    return false;
  }

  return true;
}

LCH_Json *LCH_PatchParse(const char *const raw_buffer,
                         const size_t raw_length) {
  LCH_Json *const patch = LCH_JsonParse(raw_buffer, raw_length);
  if (patch == NULL) {
    return NULL;
  }

  size_t version;
  if (!LCH_PatchGetVersion(patch, &version)) {
    LCH_JsonDestroy(patch);
    return NULL;
  }

  if (version > LCH_PATCH_VERSION) {
    LCH_LOG_ERROR("Unsupported patch version %zu", version);
    LCH_JsonDestroy(patch);
    return NULL;
  }
  LCH_LOG_DEBUG("Patch version number is %zu", version);

  return patch;
}

LCH_Json *LCH_PatchCreate(const char *const lastknown) {
  LCH_Json *const patch = LCH_JsonObjectCreate();
  if (patch == NULL) {
    return NULL;
  }

  {
    LCH_Json *const value = LCH_JsonNumberCreate((double)LCH_PATCH_VERSION);
    if (value == NULL) {
      LCH_JsonDestroy(patch);
      return NULL;
    }

    const LCH_Buffer *const key = LCH_BufferStaticFromString("version");
    if (!LCH_JsonObjectSet(patch, key, value)) {
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(patch);
      return NULL;
    }
  }

  {
    LCH_Buffer *const value = LCH_BufferFromString(lastknown);
    if (value == NULL) {
      LCH_JsonDestroy(patch);
      return NULL;
    }

    const LCH_Buffer *const key = LCH_BufferStaticFromString("lastknown");
    if (!LCH_JsonObjectSetString(patch, key, value)) {
      LCH_JsonDestroy(patch);
      return NULL;
    }
  }

  {
    const double value = (double)time(NULL);
    const LCH_Buffer *const key = LCH_BufferStaticFromString("timestamp");
    if (!LCH_JsonObjectSetNumber(patch, key, value)) {
      LCH_JsonDestroy(patch);
      return NULL;
    }
  }

  {
    LCH_Json *const value = LCH_JsonArrayCreate();
    if (value == NULL) {
      LCH_JsonDestroy(patch);
      return NULL;
    }

    const LCH_Buffer *const key = LCH_BufferStaticFromString("blocks");
    if (!LCH_JsonObjectSet(patch, key, value)) {
      LCH_JsonDestroy(value);
      LCH_JsonDestroy(patch);
      return NULL;
    }
  }

  return patch;
}

bool LCH_PatchAppendBlock(const LCH_Json *const patch, LCH_Json *const block) {
  const LCH_Buffer *const key = LCH_BufferStaticFromString("blocks");
  const LCH_Json *const blocks = LCH_JsonObjectGetArray(patch, key);
  if (blocks == NULL) {
    return false;
  }

  if (!LCH_JsonArrayAppend(blocks, block)) {
    return false;
  }

  return true;
}

bool LCH_PatchUpdateLastKnown(const LCH_Json *const patch,
                              const char *const work_dir,
                              const char *const identifier) {
  const LCH_Buffer *const key = LCH_BufferStaticFromString("lastknown");
  const LCH_Buffer *const value = LCH_JsonObjectGetString(patch, key);
  if (value == NULL) {
    return false;
  }

  const char *const lastknown = LCH_BufferData(value);
  if (!LCH_HeadSet(identifier, work_dir, lastknown)) {
    return false;
  }

  return true;
}
