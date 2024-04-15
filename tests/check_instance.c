#include <check.h>

#include "../lib/instance.h"

START_TEST(test_LCH_InstanceLoad) { ck_assert(true); }
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
