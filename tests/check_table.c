#include <check.h>

#include "../lib/definitions.h"
#include "../lib/json.h"
#include "../lib/leech.h"
#include "../lib/table.c"

START_TEST(test_LCH_TableInfoLoad) {
  LCH_Json *const definition = LCH_JsonParse(
      "{"
      "  \"primary_fields\": [ \"name\" ],"
      "  \"subsidiary_fields\": [ \"meta\" ],"
      "  \"source\": {"
      "    \"locator\": \"Hello leech!\","
      "    \"callbacks\": \".libs/module.so\""
      "  },"
      "  \"destination\": {"
      "    \"locator\": \"Bye leech!\","
      "    \"callbacks\": \".libs/module.so\""
      "  }"
      "}");
  LCH_TableInfo *const info = LCH_TableInfoLoad("CLD", definition);
  LCH_JsonDestroy(definition);

  void *conn = strdup("Hello CFEngine");
  char ***table = (char ***)info->load_callback(conn);
  ck_assert_str_eq(table[0][0], "Hello CFEngine");
  ck_assert_ptr_null(table[0][1]);
  ck_assert_ptr_null(table[1]);
  LCH_StringArrayTableDestroy(table);

  const char *actual =
      (char *)info->begin_tx_callback(info->destination_locator);
  ck_assert_str_eq(actual, "Bye leech!");

  ck_assert(info->end_tx_callback(conn, 3));

  const char *const tid = "foo";
  const char *const col = "bar";
  const char *const val = "baz";

  free(conn);
  conn = strdup("insert");
  ck_assert(info->insert_callback(conn, tid, &col, &val));

  free(conn);
  conn = strdup("delete");
  ck_assert(info->delete_callback(conn, tid, &col, &val));

  free(conn);
  conn = strdup("update");
  ck_assert(info->update_callback(conn, tid, &col, &val));
  free(conn);

  LCH_TableInfoDestroy(info);
}
END_TEST

Suite *TableSuite(void) {
  Suite *s = suite_create("table.c");
  {
    TCase *tc = tcase_create("LCH_TableInfoLoad");
    tcase_add_test(tc, test_LCH_TableInfoLoad);
    suite_add_tcase(s, tc);
  }
  return s;
}
