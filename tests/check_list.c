#include <check.h>

#include "../leech/definitions.h"
#include "../leech/list.h"

START_TEST(test_LCH_List) {
  int i, j;

  LCH_List *list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);

  ck_assert_int_eq(LCH_ListLength(list), 0);
  for (int i = 0; i < 10; i++) {
    int *data = (int *)malloc(sizeof(int));
    ck_assert_ptr_nonnull(data);
    *data = i;
    ck_assert(LCH_ListAppend(list, (void *)data, free));
  }
  ck_assert_int_eq(LCH_ListLength(list), 10);

  for (int i = 0; i < 10; i++) {
    ck_assert_int_eq(*(int *)LCH_ListGet(list, 0), 0);
  }

  LCH_ListDestroy(list);
}
END_TEST

Suite *ListSuite(void) {
  Suite *s = suite_create("list.c");
  {
    TCase *tc = tcase_create("LCH_List*");
    tcase_add_test(tc, test_LCH_List);
    suite_add_tcase(s, tc);
  }
  return s;
}
