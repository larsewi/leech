#include <check.h>
#include <limits.h>

#include "../lib/csv.h"
#include "../lib/definitions.h"
#include "../lib/delta.c"
#include "../lib/leech.h"
#include "../lib/table.h"
#include "../lib/utils.h"

START_TEST(test_LCH_Delta) {
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

  LCH_Json *actual = LCH_DeltaCreate("beatles", new_state, old_state);
  LCH_JsonDestroy(new_state);
  LCH_JsonDestroy(old_state);
  ck_assert_ptr_nonnull(actual);

  LCH_Json *expected = LCH_JsonParse(
      "{"
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
  ck_assert(LCH_JsonIsEqual(actual, expected));

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
  return s;
}
