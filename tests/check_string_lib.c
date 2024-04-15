#include <check.h>
#include <float.h>
#include <limits.h>

#include "../lib/definitions.h"
#include "../lib/dict.h"
#include "../lib/files.h"
#include "../lib/json.h"
#include "../lib/leech.h"
#include "../lib/string_lib.h"

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
  LCH_List *list = LCH_StringSplit("1.2.3", ".");
  ck_assert_int_eq(LCH_ListLength(list), 3);
  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "1");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "2");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "3");
  LCH_ListDestroy(list);
}
END_TEST

START_TEST(test_LCH_StringParseNumber) {
  long value;
  ck_assert(LCH_StringParseNumber("123", &value));
  ck_assert_int_eq(value, 123);

  ck_assert(LCH_StringParseNumber("321abc", &value));
  ck_assert_int_eq(value, 321);

  ck_assert(!LCH_StringParseNumber("abc321", &value));
}
END_TEST

START_TEST(test_LCH_StringParseVersion) {
  size_t v_major, v_minor, v_patch;
  ck_assert(LCH_StringParseVersion("1.2.3", &v_major, &v_minor, &v_patch));
  ck_assert_int_eq(v_major, 1);
  ck_assert_int_eq(v_minor, 2);
  ck_assert_int_eq(v_patch, 3);

  ck_assert(!LCH_StringParseVersion("1.2.", &v_major, &v_minor, &v_patch));
  ck_assert(!LCH_StringParseVersion("1.2", &v_major, &v_minor, &v_patch));
  ck_assert(!LCH_StringParseVersion("1.", &v_major, &v_minor, &v_patch));
  ck_assert(!LCH_StringParseVersion("1", &v_major, &v_minor, &v_patch));
  ck_assert(!LCH_StringParseVersion("", &v_major, &v_minor, &v_patch));
  ck_assert(!LCH_StringParseVersion("a.b.c", &v_major, &v_minor, &v_patch));
}
END_TEST

START_TEST(test_LCH_StringTruncate) {
  {
    const char *const str = "Very long string!";
    char *const truncated = LCH_StringTruncate(str, strlen(str), 8);
    ck_assert_str_eq(truncated, "Very ...");
    free(truncated);
  }
  {
    const char *const str = "Very long string!";
    char *const truncated = LCH_StringTruncate(str, strlen(str), 32);
    ck_assert_str_eq(truncated, "Very long string!");
    free(truncated);
  }
}
END_TEST

Suite *StringLibSuite(void) {
  Suite *s = suite_create("string_lib.c");
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
    TCase *tc = tcase_create("LCH_StringParseNumber");
    tcase_add_test(tc, test_LCH_StringParseNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_StringParseVersion");
    tcase_add_test(tc, test_LCH_StringParseVersion);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_StringTruncate");
    tcase_add_test(tc, test_LCH_StringTruncate);
    suite_add_tcase(s, tc);
  }
  return s;
}
