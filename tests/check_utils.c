#include <check.h>

#include "../leech/definitions.h"
#include "../leech/utils.h"

START_TEST(test_LCH_SplitString) {
  const char strs[][LCH_BUFFER_SIZE] = {"one two\tthree", "\t one two\tthree",
                                        "one two\tthree\t ",
                                        " \t one two \tthree\t\t "};

  for (int i = 0; i < LCH_LENGTH(strs); i++) {
    LCH_List *list = LCH_SplitString(strs[i], " \t");
    ck_assert_ptr_nonnull(list);
    ck_assert_int_eq(LCH_ListLength(list), 3);
    ck_assert_str_eq((char *)LCH_ListGet(list, 0), "one");
    ck_assert_str_eq((char *)LCH_ListGet(list, 1), "two");
    ck_assert_str_eq((char *)LCH_ListGet(list, 2), "three");
    LCH_ListDestroy(list);
  }

  LCH_List *list = LCH_SplitString("", " \t");
  ck_assert_ptr_nonnull(list);
  ck_assert_int_eq(LCH_ListLength(list), 0);
  LCH_ListDestroy(list);
}
END_TEST

Suite *UtilsSuite(void) {
  Suite *s = suite_create("utils");
  {
    TCase *tc = tcase_create("LCH_SplitString");
    tcase_add_test(tc, test_LCH_SplitString);
    suite_add_tcase(s, tc);
  }
  return s;
}
