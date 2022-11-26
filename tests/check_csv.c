#include <check.h>

#include "../leech/csv.c"

START_TEST(test_NonEscaped) {
  char buffer[] = "\"MC Cartney, Paul\",1942,\"5'11\"\"\r\n";
  size_t from = 0;
  char *non_escaped = NonEscaped(buffer, sizeof(buffer), &from);
  ck_assert_ptr_null(non_escaped);

  from = 19;
  non_escaped = NonEscaped(buffer, sizeof(buffer), &from);
  ck_assert_ptr_nonnull(non_escaped);
  ck_assert_str_eq(non_escaped, "1942");
  free(non_escaped);

  from = 23;
  non_escaped = NonEscaped(buffer, sizeof(buffer), &from);
  ck_assert_ptr_null(non_escaped);

  from = 24;
  non_escaped = NonEscaped(buffer, sizeof(buffer), &from);
  ck_assert_ptr_null(non_escaped);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s = suite_create("CSV");
  {
    TCase *tc = tcase_create("NonEscaped");
    tcase_add_test(tc, test_NonEscaped);
    suite_add_tcase(s, tc);
  }
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
