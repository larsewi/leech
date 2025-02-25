#include <check.h>
#include <string.h>

#include "../lib/definitions.h"
#include "../lib/list.h"

START_TEST(test_LCH_List) {
  LCH_List *list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);

  ck_assert_int_eq(LCH_ListLength(list), 0);
  for (int i = 0; i < 10; i++) {
    int *data = (int *)malloc(sizeof(int));
    ck_assert_ptr_nonnull(data);
    *data = i;
    ck_assert(LCH_ListAppend(list, data, free));
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

START_TEST(test_LCH_ListSortSingle) {
  LCH_List *list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);
  ck_assert(LCH_ListAppend(list, strdup("Hello World!"), free));
  LCH_ListSort(list, (int (*)(const void *, const void *))strcmp);
  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "Hello World!");
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

  ck_assert_int_eq(LCH_ListIndex(list, "George",
                                 (int (*)(const void *, const void *))strcmp),
                   2);
  ck_assert_int_eq(LCH_ListIndex(list, "Lennon",
                                 (int (*)(const void *, const void *))strcmp),
                   4);
  ck_assert_int_eq(
      LCH_ListIndex(list, "Paul", (int (*)(const void *, const void *))strcmp),
      0);
  ck_assert_int_eq(LCH_ListIndex(list, "Unknown",
                                 (int (*)(const void *, const void *))strcmp),
                   5);

  LCH_ListDestroy(list);
}
END_TEST

START_TEST(test_LCH_ListReverse) {
  LCH_List *list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);

  LCH_ListReverse(list);

  const char *strs[] = {"one", "two", "three", "four"};

  for (size_t i = 0; i < LCH_LENGTH(strs); i++) {
    ck_assert(LCH_ListAppend(list, (void *)strs[i], NULL));
  }

  LCH_ListReverse(list);

  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "four");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "three");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "two");
  ck_assert_str_eq((char *)LCH_ListGet(list, 3), "one");

  ck_assert(LCH_ListRemove(list, 0));

  LCH_ListReverse(list);

  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "one");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "two");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "three");

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
    TCase *tc = tcase_create("LCH_ListSortSingle");
    tcase_add_test(tc, test_LCH_ListSortSingle);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_ListIndex");
    tcase_add_test(tc, test_LCH_ListIndex);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_ListReverse");
    tcase_add_test(tc, test_LCH_ListReverse);
    suite_add_tcase(s, tc);
  }
  return s;
}
