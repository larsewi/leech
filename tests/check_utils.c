#include <check.h>

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

START_TEST(test_LCH_SHA1) {
  unsigned char message1[] = "Hello World!";
  unsigned char message2[] = "Hello world!";

  unsigned char *digest1 = LCH_SHA1(message1, sizeof(message1));
  unsigned char *digest2 = LCH_SHA1(message2, sizeof(message2));

  ck_assert_ptr_nonnull(digest1);
  ck_assert_ptr_nonnull(digest2);

  ck_assert_str_ne(digest1, digest2);

  free(digest1);
  free(digest2);
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
    TCase *tc = tcase_create("LCH_SHA1");
    tcase_add_test(tc, test_LCH_SHA1);
    suite_add_tcase(s, tc);
  }
  return s;
}
