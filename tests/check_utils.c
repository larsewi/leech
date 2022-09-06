#include <check.h>

#include "../src/definitions.h"
#include "../src/utils.h"

START_TEST(test_List) {
  int i, j;

  LCH_List *list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);

  ck_assert_int_eq(LCH_ListLength(list), 0);
  for (int i = 0; i < 10; i++) {
    int *data = malloc(sizeof(int));
    *data = i;
    ck_assert(LCH_ListAppend(list, (void *)data));
  }
  ck_assert_int_eq(LCH_ListLength(list), 10);

  for (int i = 0; i < 10; i++) {
    ck_assert_int_eq(*(int *) LCH_ListGet(list, 0), 0);
  }

  LCH_ListDestroy(list);
}
END_TEST

START_TEST(test_SplitString) {
  const char strs[][LCH_BUFFER_SIZE] = {
      "one two\tthree",
      "\t one two\tthree",
      "one two\tthree\t ",
      " \t one two \tthree\t\t ",
  };

  for (int i = 0; i < LCH_LENGTH(strs); i++) {
    LCH_List *list = LCH_SplitString(strs[i], " \t");
    ck_assert_ptr_nonnull(list);
    ck_assert_int_eq(LCH_ListLength(list), 3);
    ck_assert_str_eq((char *) LCH_ListGet(list, 0), "one");
    ck_assert_str_eq((char *) LCH_ListGet(list, 1), "two");
    ck_assert_str_eq((char *) LCH_ListGet(list, 2), "three");
    LCH_ListDestroy(list);
  }

  LCH_List *list = LCH_SplitString("", " \t");
  ck_assert_ptr_nonnull(list);
  ck_assert_int_eq(LCH_ListLength(list), 0);
  LCH_ListDestroy(list);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s = suite_create("Utils");
  {
    TCase *tc = tcase_create("List");
    tcase_add_test(tc, test_List);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("SplitString");
    tcase_add_test(tc, test_SplitString);
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
