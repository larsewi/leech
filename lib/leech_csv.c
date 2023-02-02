#include "leech_csv.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "csv.h"
#include "dict.h"
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

bool LCH_TableInsertCallbackCSV(const void *const locator,
                                const char *const primary,
                                const char *const subsidiary,
                                const LCH_Dict *const insert) {
  // INSERT INTO table_id (column1, column2, ...)
  // VALUES (value1, value2, ...)
  assert(locator != NULL);
  assert(insert != NULL);

  const char *const filename = locator;
  LCH_Dict *dict;
  if (LCH_IsRegularFile(filename)) {
    LCH_List *const table = LCH_CSVParseFile(filename);
    if (table == NULL) {
      LCH_LOG_ERROR("Failed to parse CSV file '%s'.", filename);
      return false;
    }

    dict = LCH_TableToDict(table, primary, subsidiary);
    LCH_ListDestroy(table);
  } else {
    dict = LCH_DictCreate();
  }

  if (dict == NULL) {
    return false;
  }

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
          "Attempted to insert a record with primary field(s) '%s' that "
          "already exists in table.",
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
        "Inserted record with primary field(s) '%s' and value(s) '%s' into "
        "table.");
  }
  LCH_ListDestroy(keys);

  LCH_List *const table = LCH_DictToTable(dict, primary, subsidiary);
  if (table == NULL) {
    LCH_DictDestroy(dict);
    return false;
  }
  LCH_DictDestroy(dict);

  if (!LCH_CSVComposeFile(table, filename)) {
    LCH_LOG_ERROR("Failed to compose CSV file '%s'.", filename);
    LCH_ListDestroy(table);
    return false;
  }

  LCH_ListDestroy(table);
  return true;
}

bool LCH_TableDeleteCallbackCSV(const void *const locator,
                                const LCH_List *const delete) {
  // DELETE FROM table_id
  // WHERE prim_field_1 = prim_value_1 AND prim_field_2 = prim_value_2;
  (void)locator;
  (void)delete;
  return true;
}

bool LCH_TableUpdateCallbackCSV(const void *const locator,
                                const LCH_List *const update) {
  // UPDATE table_id
  // SET sub_field_1 = sub_value_1, sub_field_2 = sub_value_2
  // WHERE prim_field_1 = prim_value_1 AND prim_field_2 = prim_value_2;
  (void)locator;
  (void)update;
  return true;
}
