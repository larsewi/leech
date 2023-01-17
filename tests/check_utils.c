#include <check.h>
#include <limits.h>

#include "../lib/definitions.h"
#include "../lib/leech.h"
#include "../lib/utils.h"

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
  char *expected[] = { "+,a,b,c", "-,e,d,f" };
  LCH_List *lst = LCH_SplitString("+,a,b,c\r\n-,e,d,f\r\n", "\r\n");
  ck_assert_str_eq(LCH_ListGet(lst, 0), expected[0]);
  ck_assert_str_eq(LCH_ListGet(lst, 1), expected[1]);
  LCH_ListDestroy(lst);
}
END_TEST

START_TEST(test_LCH_SplitStringSubstring) {
  {
    char *expected[] = { "+,a,b,c\r\n-,e,d,f", "%%,g,h,i" };
    LCH_List *lst = LCH_SplitStringSubstring("+,a,b,c\r\n-,e,d,f\r\n\r\n%%,g,h,i", "\r\n\r\n");
    ck_assert_str_eq(LCH_ListGet(lst, 0), expected[0]);
    ck_assert_str_eq(LCH_ListGet(lst, 1), expected[1]);
    LCH_ListDestroy(lst);
  }
  {
    char *expected[] = { "+,a,b,c\r\n-,e,d,f", "%%,g,h,i" };
    LCH_List *lst = LCH_SplitStringSubstring("\r\n\r\n+,a,b,c\r\n-,e,d,f\r\n\r\n%%,g,h,i", "\r\n\r\n");
    ck_assert_str_eq(LCH_ListGet(lst, 0), expected[0]);
    ck_assert_str_eq(LCH_ListGet(lst, 1), expected[1]);
    LCH_ListDestroy(lst);
  }
  {
    char *expected[] = { "+,a,b,c\r\n-,e,d,f", "%%,g,h,i" };
    LCH_List *lst = LCH_SplitStringSubstring("+,a,b,c\r\n-,e,d,f\r\n\r\n%%,g,h,i\r\n\r\n", "\r\n\r\n");
    ck_assert_str_eq(LCH_ListGet(lst, 0), expected[0]);
    ck_assert_str_eq(LCH_ListGet(lst, 1), expected[1]);
    LCH_ListDestroy(lst);
  }
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
  ck_assert(LCH_WriteTextFile(path, expected));

  size_t size;
  char *actual = LCH_ReadTextFile("testfile", &size);
  ck_assert_str_eq(expected, actual);
  ck_assert_int_eq(size, strlen(expected));
  free(actual);
  remove(path);
}
END_TEST

Suite *UtilsSuite(void) {
  Suite *s = suite_create("utils.c");
  {
    TCase *tc = tcase_create("LCH_SplitString");
    tcase_add_test(tc, test_LCH_SplitString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_SplitStringSubstring");
    tcase_add_test(tc, test_LCH_SplitStringSubstring);
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
  return s;
}
