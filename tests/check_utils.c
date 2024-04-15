#include <check.h>
#include <float.h>
#include <limits.h>

#include "../lib/definitions.h"
#include "../lib/dict.h"
#include "../lib/files.h"
#include "../lib/json.h"
#include "../lib/leech.h"
#include "../lib/utils.c"

START_TEST(test_LCH_MessageDigest) {
  const char tests[][128] = {
      "",
      "Hello World!",
      "Leech v1.2.3",
  };
  const char expect[][41] = {
      "da39a3ee5e6b4b0d3255bfef95601890afd80709",
      "2ef7bde608ce5404e97d5f042f95f89f1c232871",
      "71f3ebe985005bf9e00d035b7dcc245bb5c48490",
  };

  LCH_Buffer *digest;
  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
    digest = LCH_BufferCreate();
    ck_assert_ptr_nonnull(digest);
    ck_assert(LCH_MessageDigest((const unsigned char *)tests[i],
                                strlen(tests[i]), digest));
    char *const actual = LCH_BufferToString(digest);
    ck_assert_ptr_nonnull(actual);
    ck_assert_str_eq(actual, expect[i]);
    free(actual);
  }
}
END_TEST

START_TEST(test_LCH_TableToJsonObject) {
  LCH_List *table = NULL;
  {
    const char *const csv =
        "firstname, lastname,  born\r\n"
        "Paul,      McCartney, 1942\r\n"
        "Ringo,     Starr,     1940\r\n"
        "John,      Lennon,    1940\r\n"
        "George,    Harrison,  1943\r\n";
    table = LCH_CSVParseTable(csv, strlen(csv));
  }
  ck_assert_ptr_nonnull(table);

  LCH_List *primary = NULL;
  {
    const char *const csv = "firstname,lastname";
    primary = LCH_CSVParseRecord(csv, strlen(csv));
  }
  ck_assert_ptr_nonnull(primary);

  LCH_List *subsidiary = NULL;
  {
    const char *const csv = "born";
    subsidiary = LCH_CSVParseRecord(csv, strlen(csv));
  }
  ck_assert_ptr_nonnull(subsidiary);

  LCH_Json *const json = LCH_TableToJsonObject(table, primary, subsidiary);
  ck_assert_ptr_nonnull(json);
  ck_assert(LCH_JsonIsObject(json));

  LCH_ListDestroy(subsidiary);
  LCH_ListDestroy(primary);
  LCH_ListDestroy(table);

  const LCH_Buffer *str = LCH_JsonObjectGetString(
      json, LCH_BufferStaticFromString("Paul,McCartney"));
  ck_assert_str_eq(LCH_BufferData(str), "1942");

  str =
      LCH_JsonObjectGetString(json, LCH_BufferStaticFromString("Ringo,Starr"));
  ck_assert_str_eq(LCH_BufferData(str), "1940");

  str =
      LCH_JsonObjectGetString(json, LCH_BufferStaticFromString("John,Lennon"));
  ck_assert_str_eq(LCH_BufferData(str), "1940");

  str = LCH_JsonObjectGetString(json,
                                LCH_BufferStaticFromString("George,Harrison"));
  ck_assert_str_eq(LCH_BufferData(str), "1943");

  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_TableToJsonObjectNoSubsidiary) {
  LCH_List *table = NULL;
  {
    const char *const csv =
        "firstname, lastname,  born\r\n"
        "Paul,      McCartney, 1942\r\n"
        "Ringo,     Starr,     1940\r\n"
        "John,      Lennon,    1940\r\n"
        "George,    Harrison,  1943\r\n";
    table = LCH_CSVParseTable(csv, strlen(csv));
  }
  ck_assert_ptr_nonnull(table);

  LCH_List *primary = NULL;
  {
    const char *const csv = "firstname,lastname,born";
    primary = LCH_CSVParseRecord(csv, strlen(csv));
  }
  ck_assert_ptr_nonnull(primary);

  LCH_List *const subsidiary = LCH_ListCreate();
  ck_assert_ptr_nonnull(subsidiary);

  LCH_Json *const json = LCH_TableToJsonObject(table, primary, subsidiary);
  ck_assert_ptr_nonnull(json);
  ck_assert(LCH_JsonIsObject(json));

  LCH_ListDestroy(subsidiary);
  LCH_ListDestroy(primary);
  LCH_ListDestroy(table);

  const LCH_Buffer *str = LCH_JsonObjectGetString(
      json, LCH_BufferStaticFromString("Paul,McCartney,1942"));
  ck_assert_str_eq(LCH_BufferData(str), "");

  str = LCH_JsonObjectGetString(json,
                                LCH_BufferStaticFromString("Ringo,Starr,1940"));
  ck_assert_str_eq(LCH_BufferData(str), "");

  str = LCH_JsonObjectGetString(json,
                                LCH_BufferStaticFromString("John,Lennon,1940"));
  ck_assert_str_eq(LCH_BufferData(str), "");

  str = LCH_JsonObjectGetString(
      json, LCH_BufferStaticFromString("George,Harrison,1943"));
  ck_assert_str_eq(LCH_BufferData(str), "");

  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_DoubleToSize) {
  {
    size_t size = 1337;
    ck_assert(LCH_DoubleToSize(0.0, &size));
    ck_assert_int_eq(size, 0);
  }
  {
    size_t size = 1337;
    ck_assert(LCH_DoubleToSize(0.0001, &size));
    ck_assert_int_eq(size, 0);
  }
  {
    size_t size = 1337;
    ck_assert(LCH_DoubleToSize(0.9999, &size));
    ck_assert_int_eq(size, 0);
  }
  {
    size_t size = 1337;
    ck_assert(!LCH_DoubleToSize(DBL_MAX, &size));
    ck_assert_int_eq(size, 1337);
  }
  {
    size_t size = 1337;
    ck_assert(!LCH_DoubleToSize(-1.0, &size));
    ck_assert_int_eq(size, 1337);
  }
  {
    size_t size = 1337;
    ck_assert(!LCH_DoubleToSize(0.0 / 0.0, &size));
    ck_assert_int_eq(size, 1337);
  }
  {
    size_t size = 1337;
    ck_assert(!LCH_DoubleToSize(1.0 / 0.0, &size));
    ck_assert_int_eq(size, 1337);
  }
  {
    size_t size = 1337;
    ck_assert(!LCH_DoubleToSize(-1.0 / 0.0, &size));
    ck_assert_int_eq(size, 1337);
  }
}
END_TEST

Suite *UtilsSuite(void) {
  Suite *s = suite_create("utils.c");
  {
    TCase *tc = tcase_create("LCH_MessageDigest");
    tcase_add_test(tc, test_LCH_MessageDigest);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_TableToJsonObject");
    tcase_add_test(tc, test_LCH_TableToJsonObject);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_TableToJsonObject no subsidiary key");
    tcase_add_test(tc, test_LCH_TableToJsonObjectNoSubsidiary);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_DoubleToSize");
    tcase_add_test(tc, test_LCH_DoubleToSize);
    suite_add_tcase(s, tc);
  }
  return s;
}
