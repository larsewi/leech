#include <check.h>

#include "../lib/instance.c"
#include "../lib/list.h"

START_TEST(test_LCH_InstanceLoad) {
  LCH_Instance *const instance = LCH_InstanceLoad(".");
  ck_assert_ptr_nonnull(instance);
  ck_assert_int_eq(instance->major, 1);
  ck_assert_int_eq(instance->minor, 2);
  ck_assert_int_eq(instance->patch, 3);
  ck_assert_str_eq(instance->work_dir, ".");
  ck_assert_ptr_nonnull(instance->tables);
  ck_assert_int_eq(LCH_ListLength(instance->tables), 6);
  LCH_InstanceDestroy(instance);
}
END_TEST

Suite *InstanceSuite(void) {
  Suite *s = suite_create("table.c");
  {
    TCase *tc = tcase_create("LCH_InstanceLoad");
    tcase_add_test(tc, test_LCH_InstanceLoad);
    suite_add_tcase(s, tc);
  }
  return s;
}
