#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "instance.h"
#include "table.h"

struct LCH_Instance {
  char *instanceID;
  char *workDir;
  LCH_Table **tables;
  size_t numTables;
};

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->instanceID != NULL);
  assert(createInfo->workDir != NULL);

  LCH_Instance *instance = (LCH_Instance *)malloc(sizeof(LCH_Instance));
  if (instance == NULL) {
    return NULL;
  }

  instance->instanceID = createInfo->instanceID;
  instance->workDir = createInfo->workDir;
  instance->tables = NULL;
  instance->numTables = 0;

  return instance;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  free(instance->instanceID);
  free(instance->workDir);
  for (size_t i = 0; i < instance->numTables; i++) {
    LCH_TableDestroy(instance->tables[i]);
  }
  free(instance);
}
