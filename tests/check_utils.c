#include <check.h>

#include "../src/utils.h"
#include "../src/definitions.h"

START_TEST(test_Array) {
  int i, j;

  LCH_Array *array = LCH_ArrayCreate();
  ck_assert_ptr_nonnull(array);
  ck_assert_int_eq(LCH_ArrayLength(array), 0);

  ck_assert(LCH_ArrayAppendArray(array, LCH_ArrayCreate()));
  // TODO: ck_assert(LCH_ArrayAppendObject(array, LCH_ObjectCreate()));
  ck_assert(LCH_ArrayAppendString(array, strdup("Hello World!")));
  ck_assert(LCH_ArrayAppendNumber(array, 1337));
  ck_assert(LCH_ArrayAppendBoolean(array, true));

  {
    LCH_Array *get = LCH_ArrayGetArray(array, i++);
    ck_assert_ptr_nonnull(get);
    ck_assert_int_eq(LCH_ArrayLength(get), 0);
  }

  // TODO
  // {
  //   LCH_Object *get = LCH_ArrayGetObject(array, 1);
  //   ck_assert_ptr_nonnull(get);
  // }

  {
    char *get = LCH_ArrayGetString(array, i++);
    ck_assert_ptr_nonnull(get);
    ck_assert_str_eq(get, "Hello World!");
  }

  {
    long get = LCH_ArrayGetNumber(array, i++);
    ck_assert_int_eq(get, 1337);
  }

  {
    bool get = LCH_ArrayGetBoolean(array, i++);
    ck_assert(get);
  }

  /* Test realloc array functionallity */
  for (j = 0; j < 10; j++) {
    ck_assert(LCH_ArrayAppendNumber(array, i));
  }
  ck_assert_int_eq(LCH_ArrayLength(array), i + j);

  LCH_ArrayDestroy(array);
}
END_TEST

START_TEST(test_SplitString) {
  const char strs[][LCH_BUFFER_SIZE] = {
    "one two\tthree",
    "\t one two\tthree",
    "one two\tthree\t ",
    " \t one two \tthree\t\t ",
  };

  for (int i = 0; i < LCH_LENGTH(strs); i++) {
    LCH_Array *array = LCH_SplitString(strs[i], " \t");
    ck_assert_ptr_nonnull(array);
    ck_assert_int_eq(LCH_ArrayLength(array), 3);
    ck_assert_str_eq(LCH_ArrayGetString(array, 0), "one");
    ck_assert_str_eq(LCH_ArrayGetString(array, 1), "two");
    ck_assert_str_eq(LCH_ArrayGetString(array, 2), "three");
    LCH_ArrayDestroy(array);
  }

  LCH_Array *array = LCH_SplitString("", " \t");
  ck_assert_ptr_nonnull(array);
  ck_assert_int_eq(LCH_ArrayLength(array), 0);
  LCH_ArrayDestroy(array);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s = suite_create("Utils");
  {
    TCase *tc = tcase_create("Array");
    tcase_add_test(tc, test_Array);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("SplitString");
    tcase_add_test(tc, test_SplitString);
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
