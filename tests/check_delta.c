#include <check.h>
#include <limits.h>

#include "../lib/csv.h"
#include "../lib/definitions.h"
#include "../lib/delta.c"
#include "../lib/leech.h"
#include "../lib/leech_csv.h"
#include "../lib/table.h"
#include "../lib/utils.h"

START_TEST(test_LCH_Delta) {
  LCH_Buffer *out_buf = LCH_BufferCreate();
  ck_assert_ptr_nonnull(out_buf);

  LCH_TableDefinitionCreateInfo table_create_info = {
      .identifier = "beatles",
      .primary_fields = "firstname,lastname",
      .subsidiary_fields = "year",
      .read_locator = "test",
      .write_locator = "test",
      .read_callback = LCH_TableReadCallbackCSV,
      .write_callback = LCH_TableWriteCallbackCSV,
      .insert_callback = LCH_TableInsertCallbackCSV,
      .delete_callback = LCH_TableDeleteCallbackCSV,
      .update_callback = LCH_TableUpdateCallbackCSV,
  };
  LCH_TableDefinition *beatles = LCH_TableDefinitionCreate(&table_create_info);
  ck_assert_ptr_nonnull(beatles);

  table_create_info.identifier = "pinkfloyd";
  table_create_info.primary_fields = "id";
  table_create_info.subsidiary_fields = "firstname,lastname";
  LCH_TableDefinition *pinkfloyd =
      LCH_TableDefinitionCreate(&table_create_info);
  ck_assert_ptr_nonnull(pinkfloyd);

  LCH_InstanceCreateInfo instance_create_info = {
      .work_dir = ".leech",
  };

  LCH_Instance *instance = LCH_InstanceCreate(&instance_create_info);
  ck_assert_ptr_nonnull(instance);

  ck_assert(LCH_InstanceAddTableDefinition(instance, beatles));
  ck_assert(LCH_InstanceAddTableDefinition(instance, pinkfloyd));

  LCH_Dict *old_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(old_state);
  ck_assert(LCH_DictSet(old_state, "Paul,McCartney", (char *)"1942", NULL));
  ck_assert(LCH_DictSet(old_state, "Ringo,Starr", (char *)"1940", NULL));
  ck_assert(LCH_DictSet(old_state, "John,Lennon", (char *)"1940", NULL));

  LCH_Dict *new_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(new_state);
  ck_assert(LCH_DictSet(new_state, "Paul,McCartney", (char *)"1942", NULL));
  ck_assert(LCH_DictSet(new_state, "Ringo,Starr", (char *)"1941", NULL));
  ck_assert(LCH_DictSet(new_state, "George,Harrison", (char *)"1943", NULL));

  LCH_Delta *delta = LCH_DeltaCreate(beatles, new_state, old_state);
  ck_assert_ptr_nonnull(delta);
  LCH_DictDestroy(old_state);
  LCH_DictDestroy(new_state);

  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);

  ck_assert(LCH_DeltaMarshal(out_buf, delta));
  LCH_DeltaDestroy(delta);

  old_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(old_state);
  ck_assert(LCH_DictSet(old_state, "0", (char *)"Syd,Barret", NULL));
  ck_assert(LCH_DictSet(old_state, "1", (char *)"Nick,Mason", NULL));
  ck_assert(LCH_DictSet(old_state, "3", (char *)"Richard,Wright", NULL));
  ck_assert(LCH_DictSet(old_state, "4", (char *)"David,Gilmour", NULL));

  new_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(new_state);
  ck_assert(LCH_DictSet(new_state, "0", (char *)"Sid,Barretino", NULL));
  ck_assert(LCH_DictSet(new_state, "2", (char *)"Roger,Waters", NULL));
  ck_assert(LCH_DictSet(new_state, "3", (char *)"Richard,Wright", NULL));

  delta = LCH_DeltaCreate(pinkfloyd, new_state, old_state);
  ck_assert_ptr_nonnull(delta);
  LCH_DictDestroy(old_state);
  LCH_DictDestroy(new_state);

  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 2);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);

  ck_assert(LCH_DeltaMarshal(out_buf, delta));
  LCH_DeltaDestroy(delta);

  const char *in_buf = (const char *)LCH_BufferGet(out_buf, 0);

  in_buf = LCH_DeltaUnmarshal(&delta, instance, in_buf);
  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);
  ck_assert_str_eq(LCH_TableDefinitionGetIdentifier(LCH_DeltaGetTable(delta)),
                   "beatles");
  LCH_DeltaDestroy(delta);

  in_buf = LCH_DeltaUnmarshal(&delta, instance, in_buf);
  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 2);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);
  ck_assert_str_eq(LCH_TableDefinitionGetIdentifier(LCH_DeltaGetTable(delta)),
                   "pinkfloyd");
  LCH_DeltaDestroy(delta);

  LCH_InstanceDestroy(instance);
  LCH_BufferDestroy(out_buf);
}
END_TEST

START_TEST(test_LCH_DeltaV2) {
  LCH_List *primary_fields = LCH_CSVParseRecord("lastname,firstname");
  ck_assert_ptr_nonnull(primary_fields);
  LCH_List *subsidiary_fields = LCH_CSVParseRecord("born");
  ck_assert_ptr_nonnull(subsidiary_fields);

  LCH_Json *new_state = NULL;
  {
    LCH_List *table = LCH_CSVParseTable(
        "firstname,lastname,born\r\n"
        "Paul,McCartney,1942\r\n"
        "Ringo,Starr,1941\r\n"
        "John,Lennon,1940\r\n");
    ck_assert_ptr_nonnull(table);
    new_state = LCH_TableToJsonObject(table, primary_fields, subsidiary_fields);
    LCH_ListDestroy(table);
  }

  LCH_Json *old_state = NULL;
  {
    LCH_List *table = LCH_CSVParseTable(
        "firstname,lastname,born\r\n"
        "Paul,McCartney,1942\r\n"
        "Ringo,Starr,1940\r\n"
        "George,Harrison,1943\r\n");
    ck_assert_ptr_nonnull(table);
    old_state = LCH_TableToJsonObject(table, primary_fields, subsidiary_fields);
    LCH_ListDestroy(table);
  }

  LCH_ListDestroy(primary_fields);
  LCH_ListDestroy(subsidiary_fields);

  LCH_Json *actual = LCH_DeltaCreateV2("beatles", new_state, old_state);
  LCH_JsonDestroy(new_state);
  LCH_JsonDestroy(old_state);
  ck_assert_ptr_nonnull(actual);

  LCH_Json *expected = LCH_JsonParse(
      "{"
      "  \"version\": \"1.0.0\","
      "  \"type\": \"delta\","
      "  \"id\": \"beatles\","
      "  \"inserts\": {"
      "    \"Lennon,John\": \"1940\""
      "  },"
      "  \"deletes\": {"
      "    \"Harrison,George\": \"1943\""
      "  },"
      "  \"updates\": {"
      "    \"Starr,Ringo\": \"1941\""
      "  }"
      "}");

  char *json = LCH_JsonCompose(actual);
  LCH_LOG_INFO("actual:   '%s'", json);
  free(json);
  json = LCH_JsonCompose(expected);
  LCH_LOG_INFO("expected: '%s'", json);
  free(json);
  ck_assert(LCH_JsonEqual(actual, expected));

  LCH_JsonDestroy(actual);
  LCH_JsonDestroy(expected);
}
END_TEST

Suite *DeltaSuite(void) {
  Suite *s = suite_create("delta.c");
  {
    TCase *tc = tcase_create("LCH_Delta");
    tcase_add_test(tc, test_LCH_Delta);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_DeltaV2");
    tcase_add_test(tc, test_LCH_DeltaV2);
    suite_add_tcase(s, tc);
  }
  return s;
}
