#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "leech.h"

struct LCH_Instance {
  const char *instanceID;
  const char *workDir;
  LCH_List *tables;
};

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->instanceID != NULL);
  assert(createInfo->workDir != NULL);

  LCH_Instance *instance = (LCH_Instance *)malloc(sizeof(LCH_Instance));
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for instance: %s", strerror(errno));
    return NULL;
  }

  instance->instanceID = createInfo->instanceID;
  instance->workDir = createInfo->workDir;
  instance->tables = LCH_ListCreate();

  return instance;
}

LCH_List *LCH_InstanceGetTables(const LCH_Instance *const instance) {
  return instance->tables;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  LCH_ListDestroy(instance->tables);
  free(instance);
}
