#include <check.h>

#include "../src/utils.h"

START_TEST(test_LCH_Array) {
  int i, j;

  LCH_Array *array = LCH_ArrayCreate();
  ck_assert_int_eq(LCH_ArrayLength(array), 0);

  LCH_ArrayAppendArray(array, LCH_ArrayCreate());
  // TODO: LCH_ArrayAppendObject(array, LCH_ObjectCreate());
  LCH_ArrayAppendString(array, strdup("Hello World!"));
  LCH_ArrayAppendNumber(array, 1337);
  LCH_ArrayAppendBoolean(array, true);

  {
    LCH_Array *get = NULL;
    LCH_ArrayGetArray(array, i++, &get);
    ck_assert_ptr_nonnull(get);
    ck_assert_int_eq(LCH_ArrayLength(get), 0);
  }

  // TODO
  // {
  //   LCH_Object *get = NULL;
  //   LCH_ArrayGetObject(array, 1, &get);
  //   ck_assert_ptr_nonnull(get);
  // }

  {
    char *get = NULL;
    LCH_ArrayGetString(array, i++, &get);
    ck_assert_str_eq(get, "Hello World!");
  }

  {
    long get = -1;
    LCH_ArrayGetNumber(array, i++, &get);
    ck_assert_int_eq(get, 1337);
  }

  {
    bool get = false;
    LCH_ArrayGetBoolean(array, i++, &get);
    ck_assert(get);
  }

  /* Test realloc array functionallity */
  for (j = 0; j < 10; j++) {
    LCH_ArrayAppendNumber(array, i);
  }
  ck_assert_int_eq(LCH_ArrayLength(array), i + j);

  LCH_ArrayDestroy(array);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s = suite_create("Utils");
  TCase *tc = tcase_create("Array");
  tcase_add_test(tc, test_LCH_Array);
  suite_add_tcase(s, tc);
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
