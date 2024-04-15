
#include <check.h>
#include <limits.h>

#include "../lib/files.h"

START_TEST(test_LCH_FilePathJoin) {
  char path[PATH_MAX];
  ck_assert(LCH_FilePathJoin(path, sizeof(path), 3, ".leech", "snapshots",
                             "beatles"));
  ck_assert_str_eq(path, ".leech/snapshots/beatles");
}
END_TEST

Suite *FilesSuite(void) {
  Suite *s = suite_create("utils.c");
  {
    TCase *tc = tcase_create("LCH_FilePathJoin");
    tcase_add_test(tc, test_LCH_FilePathJoin);
    suite_add_tcase(s, tc);
  }
  return s;
}
