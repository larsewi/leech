#include <check.h>
#include <limits.h>

#include "../lib/definitions.h"
#include "../lib/dict.h"
#include "../lib/json.h"
#include "../lib/leech.h"
#include "../lib/utils.c"

START_TEST(test_LCH_StringEqual) {
  ck_assert(LCH_StringEqual("one", "one"));
  ck_assert(!LCH_StringEqual("one", "two"));
  ck_assert(!LCH_StringEqual("two", "one"));
  ck_assert(LCH_StringEqual("two", "two"));
}
END_TEST

START_TEST(test_LCH_StartsWith) {
  ck_assert(LCH_StringStartsWith("Hello World", "Hello"));
  ck_assert(!LCH_StringStartsWith("World", "Hello"));
  ck_assert(!LCH_StringStartsWith("Hello", "Hello World"));
  ck_assert(!LCH_StringStartsWith("", "Hello World"));
  ck_assert(LCH_StringStartsWith("Hello World", ""));
}
END_TEST

START_TEST(test_LCH_StringStrip) {
  char test1[] = "Hello World";
  ck_assert_str_eq(LCH_StringStrip(test1, " "), "Hello World");
  char test2[] = " \tHello\tWorld";
  ck_assert_str_eq(LCH_StringStrip(test2, " \t"), "Hello\tWorld");
  char test3[] = "Hello World\t";
  ck_assert_str_eq(LCH_StringStrip(test3, "\t "), "Hello World");
  char test4[] = " Hello World ";
  ck_assert_str_eq(LCH_StringStrip(test4, " "), "Hello World");
  char test5[] = "   Hello World     ";
  ck_assert_str_eq(LCH_StringStrip(test5, " "), "Hello World");
}
END_TEST

START_TEST(test_LCH_SplitString) {
  LCH_List *list = LCH_SplitString("1.2.3", ".");
  ck_assert_int_eq(LCH_ListLength(list), 3);
  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "1");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "2");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "3");
  LCH_ListDestroy(list);
}
END_TEST

START_TEST(test_LCH_PathJoin) {
  char path[PATH_MAX];
  ck_assert(
      LCH_PathJoin(path, sizeof(path), 3, ".leech", "snapshots", "beatles"));
  ck_assert_str_eq(path, ".leech/snapshots/beatles");
}
END_TEST

START_TEST(test_LCH_ReadWriteTextFile) {
  char path[] = "testfile";

  char expected[] = "Hello World!";
  ck_assert(LCH_FileWrite(path, expected));
  ck_assert(LCH_FileExists(path));

  size_t size;
  char *actual = LCH_FileRead("testfile", &size);
  ck_assert_str_eq(expected, actual);
  ck_assert_int_eq(size, strlen(expected));
  free(actual);
  remove(path);
  ck_assert(!LCH_FileExists(path));
}
END_TEST

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
  LCH_List *const table = LCH_CSVParseTable(
      "firstname, lastname,  born\r\n"
      "Paul,      McCartney, 1942\r\n"
      "Ringo,     Starr,     1940\r\n"
      "John,      Lennon,    1940\r\n"
      "George,    Harrison,  1943\r\n");
  static const char *primary[] = {"firstname", "lastname", NULL};
  static const char *subsidiary[] = {"born", NULL};

  LCH_Json *const json = LCH_TableToJsonObject(table, primary, subsidiary);
  ck_assert_ptr_nonnull(json);

  LCH_ListDestroy(table);

  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_OBJECT);

  const LCH_Json *str = LCH_JsonObjectGet(json, "Paul,McCartney");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "1942");

  str = LCH_JsonObjectGet(json, "Ringo,Starr");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "1940");

  str = LCH_JsonObjectGet(json, "John,Lennon");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "1940");

  str = LCH_JsonObjectGet(json, "George,Harrison");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "1943");

  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_TableToJsonObjectNoSubsidiary) {
  LCH_List *const table = LCH_CSVParseTable(
      "firstname, lastname,  born\r\n"
      "Paul,      McCartney, 1942\r\n"
      "Ringo,     Starr,     1940\r\n"
      "John,      Lennon,    1940\r\n"
      "George,    Harrison,  1943\r\n");
  static const char *primary[] = {"born", "lastname", "firstname", NULL};
  static const char *subsidiary[] = {NULL};

  LCH_Json *const json = LCH_TableToJsonObject(table, primary, subsidiary);
  ck_assert_ptr_nonnull(json);

  LCH_ListDestroy(table);

  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_OBJECT);

  const LCH_Json *str = LCH_JsonObjectGet(json, "1942,McCartney,Paul");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "");

  str = LCH_JsonObjectGet(json, "1940,Starr,Ringo");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "");

  str = LCH_JsonObjectGet(json, "1940,Lennon,John");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "");

  str = LCH_JsonObjectGet(json, "1943,Harrison,George");
  ck_assert_str_eq(LCH_JsonStringGetString(str), "");

  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_ParseNumber) {
  long value;
  ck_assert(LCH_ParseNumber("123", &value));
  ck_assert_int_eq(value, 123);

  ck_assert(LCH_ParseNumber("321abc", &value));
  ck_assert_int_eq(value, 321);

  ck_assert(!LCH_ParseNumber("abc321", &value));
}
END_TEST

START_TEST(test_LCH_ParseVersion) {
  size_t major, minor, patch;
  ck_assert(LCH_ParseVersion("1.2.3", &major, &minor, &patch));
  ck_assert_int_eq(major, 1);
  ck_assert_int_eq(minor, 2);
  ck_assert_int_eq(patch, 3);

  ck_assert(!LCH_ParseVersion("1.2.", &major, &minor, &patch));
  ck_assert(!LCH_ParseVersion("1.2", &major, &minor, &patch));
  ck_assert(!LCH_ParseVersion("1.", &major, &minor, &patch));
  ck_assert(!LCH_ParseVersion("1", &major, &minor, &patch));
  ck_assert(!LCH_ParseVersion("", &major, &minor, &patch));
  ck_assert(!LCH_ParseVersion("a.b.c", &major, &minor, &patch));
}
END_TEST

Suite *UtilsSuite(void) {
  Suite *s = suite_create("utils.c");
  {
    TCase *tc = tcase_create("LCH_StringEqual");
    tcase_add_test(tc, test_LCH_StringEqual);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_SplitString");
    tcase_add_test(tc, test_LCH_SplitString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_StartsWith");
    tcase_add_test(tc, test_LCH_StartsWith);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_StringStrip");
    tcase_add_test(tc, test_LCH_StringStrip);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_PathJoin");
    tcase_add_test(tc, test_LCH_PathJoin);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_ReadWriteTextFile");
    tcase_add_test(tc, test_LCH_ReadWriteTextFile);
    suite_add_tcase(s, tc);
  }
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
    TCase *tc = tcase_create("LCH_ParseNumber");
    tcase_add_test(tc, test_LCH_ParseNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_ParseVersion");
    tcase_add_test(tc, test_LCH_ParseVersion);
    suite_add_tcase(s, tc);
  }
  return s;
}
