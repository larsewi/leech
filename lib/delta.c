#include "delta.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "csv.h"
#include "instance.h"
#include "leech.h"
#include "list.h"
#include "table.h"
#include "utils.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

void LCH_DeltaDestroy(void *const delta) { LCH_JsonDestroy(delta); }

LCH_Delta *LCH_DeltaCreate(const char *const table_id,
                           const LCH_Json *const new_state,
                           const LCH_Json *const old_state) {
  assert(table_id != NULL);
  assert(new_state != NULL);
  assert(old_state != NULL);

  LCH_Json *const delta = LCH_JsonObjectCreate();
  if (delta == NULL) {
    return NULL;
  }

  char *const version =
      LCH_VersionToString(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  if (version == NULL) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(delta, "version", version)) {
    free(version);
    LCH_JsonDestroy(delta);
    return NULL;
  }
  free(version);

  if (!LCH_JsonObjectSetStringDuplicate(delta, "type", "delta")) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSetStringDuplicate(delta, "id", table_id)) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  LCH_Json *const inserts = LCH_JsonObjectKeysSetMinus(new_state, old_state);
  if (inserts == NULL) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSet(delta, "inserts", inserts)) {
    LCH_JsonDestroy(inserts);
    LCH_JsonDestroy(delta);
    return NULL;
  }

  LCH_Json *const deletes = LCH_JsonObjectKeysSetMinus(old_state, new_state);
  if (deletes == NULL) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSet(delta, "deletes", deletes)) {
    LCH_JsonDestroy(deletes);
    LCH_JsonDestroy(delta);
    return NULL;
  }

  LCH_Json *const updates =
      LCH_JsonObjectKeysSetIntersectAndValuesSetMinus(new_state, old_state);
  if (updates == NULL) {
    LCH_JsonDestroy(delta);
    return NULL;
  }

  if (!LCH_JsonObjectSet(delta, "updates", updates)) {
    LCH_JsonDestroy(updates);
    LCH_JsonDestroy(delta);
    return NULL;
  }

  return delta;
}

size_t LCH_DeltaGetNumInserts(const LCH_Delta *const delta) {
  assert(delta != NULL);
  const LCH_Json *const inserts = LCH_JsonObjectGet(delta, "inserts");
  const size_t num_inserts = LCH_JsonObjectLength(inserts);
  return num_inserts;
}

size_t LCH_DeltaGetNumDeletes(const LCH_Delta *const delta) {
  assert(delta != NULL);
  const LCH_Json *const deletes = LCH_JsonObjectGet(delta, "deletes");
  const size_t num_deletes = LCH_JsonObjectLength(deletes);
  return num_deletes;
}

size_t LCH_DeltaGetNumUpdates(const LCH_Delta *const delta) {
  assert(delta != NULL);
  const LCH_Json *const updates = LCH_JsonObjectGet(delta, "updates");
  const size_t num_updates = LCH_JsonObjectLength(updates);
  return num_updates;
}

const char *LCH_DeltaGetTableID(const LCH_Delta *const delta) {
  assert(delta != NULL);
  const LCH_Json *const table_id = LCH_JsonObjectGet(delta, "id");
  const char *const str = LCH_JsonGetString(table_id);
  return str;
}
