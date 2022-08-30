#include <check.h>

#include "../src/utils.h"

START_TEST(test_LCH_ArrayCreate) {
  LCH_Array *array = LCH_ArrayCreate();
  ck_assert_ptr_nonnull(array);
  LCH_ArrayDestroy(array);
}
END_TEST

START_TEST(test_LCH_ArrayLength) {
  LCH_Array *array = LCH_ArrayCreate();
  ck_assert_int_eq(LCH_ArrayLength(array), 0);
  for (int i = 0; i < 10; i++) {
    LCH_ArrayAppendNumber(array, i);
  }
  ck_assert_int_eq(LCH_ArrayLength(array), 10);
  LCH_ArrayDestroy(array);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s = suite_create("Utils");
  TCase *tc = tcase_create("Array");
  tcase_add_test(tc, test_LCH_ArrayCreate);
  tcase_add_test(tc, test_LCH_ArrayLength);
  suite_add_tcase(s, tc);
  return s;
}

int main(void) {
  Suite *s = utils_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
