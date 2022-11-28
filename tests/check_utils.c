#include <check.h>

#include "../leech/definitions.h"
#include "../leech/utils.h"

START_TEST(test_List) {
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

START_TEST(test_Dict) {
  LCH_Dict *dict = LCH_DictCreate();
  ck_assert_ptr_nonnull(dict);

  const char keys[][LCH_BUFFER_SIZE] = {"one", "two",   "three", "four", "five",
                                        "six", "seven", "eight", "nine", "ten"};

  ck_assert_int_eq(LCH_DictLength(dict), 0);
  for (int i = 0; i < LCH_LENGTH(keys); i++) {
    int *data = (int *)malloc(sizeof(int));
    ck_assert_ptr_nonnull(data);
    *data = i;
    ck_assert(LCH_DictSet(dict, keys[i], (void *)data, free));
  }
  ck_assert_int_eq(LCH_DictLength(dict), LCH_LENGTH(keys));

  for (int i = 0; i < LCH_LENGTH(keys); i++) {
    ck_assert(LCH_DictHasKey(dict, keys[i]));
    int *data = (int *)LCH_DictGet(dict, keys[i]);
    ck_assert_int_eq(*data, i);
  }
  ck_assert(!LCH_DictHasKey(dict, "bogus"));
  ck_assert_int_eq(LCH_DictLength(dict), LCH_LENGTH(keys));

  LCH_DictDestroy(dict);
}
END_TEST

START_TEST(test_SplitString) {
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

START_TEST(test_FileWriteCSV) {
  LCH_List *table = LCH_ListCreate();

  LCH_List *record = LCH_ListCreate();
  char *str = strdup("firstname");
  LCH_ListAppend(record, (void *)str, free);
  str = strdup("lastname");
  LCH_ListAppend(record, (void *)str, free);
  str = strdup("age");
  LCH_ListAppend(record, (void *)str, free);
  LCH_ListAppend(table, (void *)record, free);

  record = LCH_ListCreate();
  str = strdup("Paul");
  LCH_ListAppend(record, (void *)str, free);
  str = strdup("MC Cartney");
  LCH_ListAppend(record, (void *)str, free);
  str = strdup("1942");
  LCH_ListAppend(record, (void *)str, free);
  LCH_ListAppend(table, (void *)record, free);

  FILE *file = fopen("beatles.csv", "w");
  bool success = LCH_FileWriteCSVTable(file, table);
  fclose(file);
  ck_assert(success);

  char expected[] = "firstname,lastname,age\r\nPaul,MC Cartney,1942";
  char actual[128];
  file = fopen("beatles.csv", "r");
  size_t bytes = fread(actual, 1, sizeof(actual), file);
  fclose(file);
  ck_assert_int_eq(bytes, strlen(expected));
  actual[bytes] = '\0';
  ck_assert_str_eq(expected, actual);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s = suite_create("Utils");
  {
    TCase *tc = tcase_create("List");
    tcase_add_test(tc, test_List);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("Dict");
    tcase_add_test(tc, test_List);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("SplitString");
    tcase_add_test(tc, test_SplitString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("FileWriteCSV");
    tcase_add_test(tc, test_FileWriteCSV);
    suite_add_tcase(s, tc);
  }
  return s;
}

int main(void) {
  Suite *s = utils_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
