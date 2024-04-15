#include <check.h>

#include "../lib/table.c"

START_TEST(test_LCH_TableInfoLoad) { ck_assert(true); }
END_TEST

Suite *TableSuite(void) {
  Suite *s = suite_create("table.c");
  {
    TCase *tc = tcase_create("LCH_TableInfoLoad");
    tcase_add_test(tc, test_LCH_TableInfoLoad);
    suite_add_tcase(s, tc);
  }
  return s;
}
