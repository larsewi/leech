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

START_TEST(test_LCH_SplitString) {}
END_TEST

START_TEST(test_LCH_PathJoin) {
  char path[PATH_MAX];
  ck_assert(
      LCH_PathJoin(path, sizeof(path), 3, ".leech", "snapshots", "beatles"));
  ck_assert_str_eq(path, ".leech/snapshots/beatles");
}
END_TEST

START_TEST(test_LCH_ReadFile) {
  char path[] = "testfile";
  FILE *file = fopen(path, "w");
  ck_assert_ptr_nonnull(file);

  char expected[] = "Hello World!";
  size_t bytes_written = fwrite(expected, 1, sizeof(expected), file);
  fclose(file);

  size_t size;
  char *actual = LCH_ReadFile("testfile", &size);
  ck_assert_str_eq(expected, actual);
  ck_assert_int_eq(size, strlen(expected) + 1);
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
    TCase *tc = tcase_create("LCH_ReadFile");
    tcase_add_test(tc, test_LCH_PathJoin);
    suite_add_tcase(s, tc);
  }
  return s;
}
