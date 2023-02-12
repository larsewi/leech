#include "leech_csv.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "csv.h"
#include "utils.h"

LCH_List *LCH_TableReadCallbackCSV(const void *const locator) {
  assert(locator != NULL);

  const char *const filename = locator;
  return LCH_CSVParseFile(filename);
}

bool LCH_TableWriteCallbackCSV(const void *const locator,
                               const LCH_List *const table) {
  assert(locator != NULL);
  assert(table != NULL);

  const char *const filename = locator;
  return LCH_CSVComposeFile(table, filename);
}

static LCH_Dict *LoadTableAsDict(const char *const filename,
                                 const char *const primary,
                                 const char *const subsidiary) {
  if (LCH_IsRegularFile(filename)) {
    LCH_List *const table = LCH_CSVParseFile(filename);
    if (table == NULL) {
      LCH_LOG_ERROR("Failed to parse CSV file '%s'.", filename);
      return NULL;
    }

    LCH_Dict *const dict = LCH_TableToDict(table, primary, subsidiary);
    LCH_ListDestroy(table);
    return dict;
  }
  LCH_Dict *const dict = LCH_DictCreate();
  return dict;
}

static bool StoreTableAsDict(const LCH_Dict *const dict,
                             const char *const filename,
                             const char *const primary,
                             const char *const subsidiary) {
  LCH_List *const table = LCH_DictToTable(dict, primary, subsidiary);
  if (table == NULL) {
    return false;
  }

  if (!LCH_CSVComposeFile(table, filename)) {
    LCH_LOG_ERROR("Failed to compose CSV file '%s'.", filename);
    LCH_ListDestroy(table);
    return false;
  }

  LCH_ListDestroy(table);
  return true;
}

bool LCH_TableInsertCallbackCSV(const void *const locator,
                                const char *const primary,
                                const char *const subsidiary,
                                const LCH_Dict *const insert) {
  assert(locator != NULL);
  assert(insert != NULL);

  const char *const filename = locator;
  LCH_Dict *dict = LoadTableAsDict(filename, primary, subsidiary);
  if (dict == NULL) {
    LCH_LOG_ERROR("Failed to load table '%s'.", filename);
    return false;
  }
  LCH_LOG_DEBUG("Loaded table '%s' containing %zu records.", filename,
                LCH_DictLength(dict));

  LCH_List *const keys = LCH_DictGetKeys(insert);
  if (keys == NULL) {
    LCH_DictDestroy(dict);
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    if (LCH_DictHasKey(dict, key)) {
      LCH_LOG_ERROR(
          "Attempted to insert already existing record with primary field(s) "
          "'%s' into table.",
          key);
      LCH_ListDestroy(keys);
      LCH_DictDestroy(dict);
      return false;
    }

    char *const value = LCH_DictGet(insert, key);
    if (!LCH_DictSet(dict, key, value, NULL)) {
      LCH_ListDestroy(keys);
      LCH_DictDestroy(dict);
      return false;
    }
    LCH_LOG_DEBUG(
        "Inserted record with primary field(s) '%s' and subsidiary field(s) "
        "'%s' into "
        "table.",
        key, value);
  }
  LCH_ListDestroy(keys);

  if (!StoreTableAsDict(dict, filename, primary, subsidiary)) {
    LCH_LOG_ERROR("Failed to store table '%s'.", filename);
    LCH_DictDestroy(dict);
    return false;
  }
  LCH_LOG_DEBUG("Stored table '%s' containing %zu records.", filename,
                LCH_DictLength(dict));
  LCH_DictDestroy(dict);

  return true;
}

bool LCH_TableDeleteCallbackCSV(const void *const locator,
                                const char *const primary,
                                const char *const subsidiary,
                                const LCH_Dict *const delete) {
  assert(locator != NULL);
  assert(delete != NULL);

  const char *const filename = locator;
  LCH_Dict *dict = LoadTableAsDict(filename, primary, subsidiary);
  if (dict == NULL) {
    LCH_LOG_ERROR("Failed to load table '%s'.", filename);
    return false;
  }
  LCH_LOG_DEBUG("Loaded table '%s' containing %zu records.", filename,
                LCH_DictLength(dict));

  LCH_List *const keys = LCH_DictGetKeys(delete);
  if (keys == NULL) {
    LCH_DictDestroy(dict);
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    if (!LCH_DictHasKey(dict, key)) {
      LCH_LOG_ERROR(
          "Attempted to delete a non-existent record with primary field(s) "
          "'%s' from table",
          key);
      LCH_ListDestroy(keys);
      LCH_DictDestroy(dict);
      return false;
    }

    char *const value = LCH_DictRemove(dict, key);
    LCH_LOG_DEBUG(
        "Deleted record with primary field(s) '%s' and subsidiary field(s) "
        "'%s' from table.",
        key, value);
    free(value);
  }

  LCH_ListDestroy(keys);

  if (!StoreTableAsDict(dict, filename, primary, subsidiary)) {
    LCH_LOG_ERROR("Failed to store table '%s'.", filename);
    LCH_DictDestroy(dict);
    return false;
  }
  LCH_LOG_DEBUG("Stored table '%s' containing %zu records.", filename,
                LCH_DictLength(dict));
  LCH_DictDestroy(dict);

  return true;
}

bool LCH_TableUpdateCallbackCSV(const void *const locator,
                                const char *const primary,
                                const char *const subsidiary,
                                const LCH_Dict *const update) {
  assert(locator != NULL);
  assert(update != NULL);

  const char *const filename = locator;
  LCH_Dict *dict = LoadTableAsDict(filename, primary, subsidiary);
  if (dict == NULL) {
    LCH_LOG_ERROR("Failed to load table '%s'.", filename);
    return false;
  }
  LCH_LOG_DEBUG("Loaded table '%s' containing %zu records.", filename,
                LCH_DictLength(dict));

  LCH_List *const keys = LCH_DictGetKeys(update);
  if (keys == NULL) {
    LCH_DictDestroy(dict);
    return false;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    if (!LCH_DictHasKey(dict, key)) {
      LCH_LOG_ERROR(
          "Attempted to update a non-existent record with primary field(s) "
          "'%s' from table",
          key);
      LCH_ListDestroy(keys);
      LCH_DictDestroy(dict);
      return false;
    }

    char *const value = LCH_DictGet(update, key);
    if (!LCH_DictSet(dict, key, value, NULL)) {
      LCH_ListDestroy(keys);
      LCH_DictDestroy(dict);
      return false;
    }
    LCH_LOG_DEBUG(
        "Updated record with primary field(s) '%s' to contain subsidiary "
        "field(s) '%s' in table.",
        key, value);
  }

  LCH_ListDestroy(keys);

  if (!StoreTableAsDict(dict, filename, primary, subsidiary)) {
    LCH_LOG_ERROR("Failed to store table '%s'.", filename);
    LCH_DictDestroy(dict);
    return false;
  }
  LCH_LOG_DEBUG("Stored table '%s' containing %zu records.", filename,
                LCH_DictLength(dict));
  LCH_DictDestroy(dict);

  return true;
}
