#include "table.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "csv.h"
#include "definitions.h"
#include "leech.h"
#include "list.h"
#include "utils.h"

typedef struct LCH_TableDefinition {
  const char *identifier;
  const char *primary_fields;
  const char *subsidiary_fields;
  const void *read_locator;
  const void *write_locator;
  LCH_List *(*read_callback)(const void *);
  bool (*write_callback)(const void *, const LCH_List *);
  bool (*insert_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*delete_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*update_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
} LCH_TableDefinition;

const char *LCH_TableDefinitionGetIdentifier(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->identifier;
}

const char *LCH_TableDefinitionGetPrimaryFields(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->primary_fields;
}

const char *LCH_TableDefinitionGetSubsidiaryFields(
    const LCH_TableDefinition *const self) {
  assert(self != NULL);
  return self->subsidiary_fields;
}

LCH_TableDefinition *LCH_TableDefinitionCreate(
    const LCH_TableDefinitionCreateInfo *const create_info) {
  assert(create_info != NULL);
  assert(create_info->identifier != NULL);
  assert(create_info->primary_fields != NULL);
  assert(create_info->read_locator != NULL);
  assert(create_info->write_locator != NULL);
  assert(create_info->read_callback != NULL);
  assert(create_info->write_callback != NULL);
  assert(create_info->insert_callback != NULL);
  assert(create_info->delete_callback != NULL);
  assert(create_info->update_callback != NULL);

  LCH_TableDefinition *definition =
      (LCH_TableDefinition *)calloc(1, sizeof(LCH_TableDefinition));
  if (definition == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  definition->identifier = create_info->identifier;
  definition->primary_fields = create_info->primary_fields;
  definition->subsidiary_fields = create_info->subsidiary_fields;
  definition->read_locator = create_info->read_locator;
  definition->write_locator = create_info->write_locator;
  definition->read_callback = create_info->read_callback;
  definition->write_callback = create_info->write_callback;
  definition->insert_callback = create_info->insert_callback;
  definition->delete_callback = create_info->delete_callback;
  definition->update_callback = create_info->update_callback;

  return definition;
}

LCH_Json *LCH_TableDefinitionLoadNewState(
    const LCH_TableDefinition *const self) {
  LCH_List *const table = self->read_callback(self->read_locator);
  if (table == NULL) {
    return NULL;
  }

  LCH_List *const primary_fields = LCH_CSVParseRecord(self->primary_fields);
  if (primary_fields == NULL) {
    LCH_ListDestroy(table);
    return NULL;
  }
  LCH_ListSort(primary_fields, (int (*)(const void *, const void *))strcmp);

  LCH_List *const subsidiary_fields =
      (self->subsidiary_fields != NULL)
          ? LCH_CSVParseRecord(self->subsidiary_fields)
          : LCH_ListCreate();
  if (subsidiary_fields == NULL) {
    LCH_ListDestroy(primary_fields);
    LCH_ListDestroy(table);
    return NULL;
  }
  LCH_ListSort(subsidiary_fields, (int (*)(const void *, const void *))strcmp);

  LCH_Json *const state =
      LCH_TableToJsonObject(table, primary_fields, subsidiary_fields);
  LCH_ListDestroy(subsidiary_fields);
  LCH_ListDestroy(primary_fields);
  LCH_ListDestroy(table);

  return state;
}

LCH_Json *LCH_TableDefinitionLoadOldState(const LCH_TableDefinition *const self,
                                          const char *const work_dir) {
  assert(self != NULL);
  assert(work_dir != NULL);
  assert(self->identifier != NULL);

  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot.json",
                    self->identifier)) {
    return NULL;
  }

  if (LCH_IsRegularFile(path)) {
    LCH_Json *const state = LCH_JsonParse(path);
    return state;
  }

  LCH_Json *const state = LCH_JsonObjectCreate();
  return state;
}

bool LCH_TableStoreNewState(const LCH_TableDefinition *const self,
                            const char *const work_dir,
                            const LCH_Json *const state) {
  char path[PATH_MAX];
  if (!LCH_PathJoin(path, sizeof(path), 3, work_dir, "snapshot.json",
                    self->identifier)) {
    return false;
  }

  char *const json = LCH_JsonCompose(state);
  if (json == NULL) {
    return false;
  }

  if (!LCH_FileWrite(path, json)) {
    free(json);
    return false;
  }
  free(json);
  return true;
}

void LCH_TableDefinitionDestroy(void *self) { free(self); }
