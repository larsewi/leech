#include <check.h>

#include "../lib/definitions.h"
#include "../lib/leech.h"
#include "../lib/leech_csv.h"
#include "../lib/table.h"

START_TEST(test_CreateDestroyTable) { ck_assert(true); }
END_TEST

Suite *TableSuite(void) {
  Suite *s = suite_create("table.c");
  {
    TCase *tc = tcase_create("Create/Destroy");
    tcase_add_test(tc, test_CreateDestroyTable);
    suite_add_tcase(s, tc);
  }
  return s;
}
