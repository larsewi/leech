#include "../src/leech.h"
#include <check.h>
#include <stdlib.h>

START_TEST(test_test) { ck_assert(1); }
END_TEST

Suite *leech_suite(void) {
  Suite *s = suite_create("Leech");

  /* Array test case */
  TCase *tc = tcase_create("Test");
  tcase_add_test(tc, test_test);
  suite_add_tcase(s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = leech_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
