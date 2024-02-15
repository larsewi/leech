
#include <check.h>
#include <sys/stat.h>

#include "../lib/definitions.h"
#include "../lib/head.c"

#define NEW_HASH "1234567890123456789012345678901234567890"

START_TEST(test_HeadGetSet) {
  char work_dir[] = ".leech";
  mkdir(work_dir, S_IRWXU);

  char *head = LCH_HeadGet(work_dir);
  ck_assert_ptr_nonnull(head);
  ck_assert_str_eq(head, "0000000000000000000000000000000000000000");

  strcpy(head, NEW_HASH);
  ck_assert(LCH_HeadSet(work_dir, head));
  free(head);

  head = LCH_HeadGet(work_dir);
  ck_assert_ptr_nonnull(head);
  ck_assert_str_eq(head, NEW_HASH);

  free(head);
}
END_TEST

Suite *HeadSuite(void) {
  Suite *s = suite_create("head.c");
  {
    TCase *tc = tcase_create("HeadGet/HeadSet");
    tcase_add_test(tc, test_HeadGetSet);
    suite_add_tcase(s, tc);
  }
  return s;
}
