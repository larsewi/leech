#include <check.h>
#include <limits.h>

#include "../lib/csv.h"
#include "../lib/delta.c"
#include "../lib/utils.h"

START_TEST(test_LCH_Delta) {
  LCH_List *const primary_fields = LCH_ListCreate();
  ck_assert_ptr_nonnull(primary_fields);
  {
    const LCH_Buffer field = LCH_BufferStaticFromString("lastname");
    ck_assert(LCH_ListAppendBufferDuplicate(primary_fields, &field));
  }
  {
    const LCH_Buffer field = LCH_BufferStaticFromString("firstname");
    ck_assert(LCH_ListAppendBufferDuplicate(primary_fields, &field));
  }

  LCH_List *const subsidiary_fields = LCH_ListCreate();
  const LCH_Buffer field = LCH_BufferStaticFromString("born");
  ck_assert(LCH_ListAppendBufferDuplicate(subsidiary_fields, &field));

  LCH_Json *new_state = NULL;
  {
    const char *const csv =
        "firstname,lastname,born\r\n"
        "Paul,McCartney,1942\r\n"
        "Ringo,Starr,1941\r\n"
        "John,Lennon,1940\r\n";
    LCH_List *table = LCH_CSVParseTable(csv, strlen(csv));
    ck_assert_ptr_nonnull(table);
    new_state = LCH_TableToJsonObject(table, primary_fields, subsidiary_fields);
    LCH_ListDestroy(table);
  }
  ck_assert_ptr_nonnull(new_state);

  LCH_Json *old_state = NULL;
  {
    const char *const csv =
        "firstname,lastname,born\r\n"
        "Paul,McCartney,1942\r\n"
        "Ringo,Starr,1940\r\n"
        "George,Harrison,1943\r\n";
    LCH_List *table = LCH_CSVParseTable(csv, strlen(csv));
    ck_assert_ptr_nonnull(table);
    old_state = LCH_TableToJsonObject(table, primary_fields, subsidiary_fields);
    LCH_ListDestroy(table);
  }
  ck_assert_ptr_nonnull(old_state);

  LCH_ListDestroy(primary_fields);
  LCH_ListDestroy(subsidiary_fields);

  LCH_Json *const actual =
      LCH_DeltaCreate("beatles", "delta", new_state, old_state);
  LCH_JsonDestroy(new_state);
  LCH_JsonDestroy(old_state);
  ck_assert_ptr_nonnull(actual);

  const char *const csv =
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
      "}";
  LCH_Json *const expected = LCH_JsonParse(csv, strlen(csv));

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
  return s;
}
