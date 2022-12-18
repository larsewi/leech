#include <check.h>
#include <string.h>

#include "../lib/definitions.h"
#include "../lib/leech.h"

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

START_TEST(test_LCH_ListSort) {
  LCH_List *list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);

  const char *strs[] = {"b", "c", "a", "ba", "ab", "ac", "aa"};
  for (size_t i = 0; i < LCH_LENGTH(strs); i++) {
    char *data = strdup(strs[i]);
    ck_assert_ptr_nonnull(data);
    ck_assert(LCH_ListAppend(list, data, free));
  }
  LCH_ListSort(list, (int (*)(const void *, const void *))strcmp);
  ck_assert_int_eq(LCH_ListLength(list), LCH_LENGTH(strs));

  const char *expect[] = {"a", "aa", "ab", "ac", "b", "ba", "c"};
  for (size_t i = 0; i < LCH_ListLength(list); i++) {
    ck_assert_str_eq((char *)LCH_ListGet(list, i), expect[i]);
  }

  LCH_ListDestroy(list);
}
END_TEST

START_TEST(test_LCH_ListIndex) {
  LCH_List *list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);

  const char *strs[] = {"Paul", "Ringo", "George", "", "Lennon"};
  for (size_t i = 0; i < LCH_LENGTH(strs); i++) {
    ck_assert(LCH_ListAppend(list, strdup(strs[i]), free));
  }

  ck_assert_int_eq(LCH_ListIndex(list, "George", (int (*)(const void *, const void *))strcmp), 2);
  ck_assert_int_eq(LCH_ListIndex(list, "Lennon", (int (*)(const void *, const void *))strcmp), 4);
  ck_assert_int_eq(LCH_ListIndex(list, "Paul", (int (*)(const void *, const void *))strcmp), 0);
  ck_assert_int_eq(LCH_ListIndex(list, "Unknown", (int (*)(const void *, const void *))strcmp), 5);

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
  {
    TCase *tc = tcase_create("LCH_ListSort");
    tcase_add_test(tc, test_LCH_ListSort);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_ListIndex");
    tcase_add_test(tc, test_LCH_ListIndex);
    suite_add_tcase(s, tc);
  }
  return s;
}
