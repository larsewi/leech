#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"
#include "leech.h"
#include "table.h"

struct LCH_Instance {
  const char *identifier;
  const char *work_dir;
  LCH_List *tables;
};

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->identifier != NULL);
  assert(createInfo->work_dir != NULL);

  LCH_Instance *instance = (LCH_Instance *)malloc(sizeof(LCH_Instance));
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for instance: %s",
                  strerror(errno));
    return NULL;
  }

  instance->identifier = createInfo->identifier;
  instance->work_dir = createInfo->work_dir;
  instance->tables = LCH_ListCreate();
  if (instance->tables == NULL) {
    free(instance);
    return NULL;
  }

  return instance;
}

bool LCH_InstanceAddTable(LCH_Instance *const instance, LCH_Table *const table) {
  assert(instance != NULL);
  assert(instance->tables != NULL);
  assert(table != NULL);

  return LCH_ListAppend(instance->tables, table, (void (*)(void *)) LCH_TableDestroy);
}

bool LCH_InstanceCommit(const LCH_Instance *const self) {
  assert(self != NULL);

  LCH_List *tables = self->tables;
  size_t num_tables = LCH_ListLength(tables);

  for (size_t i = 0; i < num_tables; i++) {
    const LCH_Table *const table = LCH_ListGet(tables, i);
    const char *const table_id = LCH_TableGetIdentifier(table);

    LCH_LOG_DEBUG("Loading new data from table '%s'", table_id);
    LCH_Dict *new_data = LCH_TableLoadNewData(table);
    if (new_data == NULL) {
      LCH_LOG_ERROR("Failed to load new data from table '%s'", table_id);
      return false;
    }

    LCH_LOG_DEBUG("Loading old data from table '%s'", table_id);
    LCH_Dict *old_data = LCH_TableLoadOldData(table, self->work_dir);
    if (old_data == NULL) {
      LCH_LOG_ERROR("Failed to load old data from table '%s'", table_id);
      LCH_DictDestroy(new_data);
      return false;
    }

    LCH_DictDestroy(new_data);
    LCH_DictDestroy(old_data);
  }

  return true;
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
