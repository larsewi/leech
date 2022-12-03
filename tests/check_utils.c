#include <check.h>

#include "../leech/definitions.h"
#include "../leech/utils.h"

START_TEST(test_LCH_StartsWith) {
  ck_assert(LCH_StartsWith("Hello World", "Hello"));
  ck_assert(!LCH_StartsWith("World", "Hello"));
  ck_assert(!LCH_StartsWith("Hello", "Hello World"));
  ck_assert(!LCH_StartsWith("", "Hello World"));
  ck_assert(LCH_StartsWith("Hello World", ""));
}
END_TEST

START_TEST(test_LCH_SplitString) {}
END_TEST

Suite *UtilsSuite(void) {
  Suite *s = suite_create("utils");
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
  return s;
}
