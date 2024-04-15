#include <check.h>
#include <stdio.h>

#include "../lib/dict.h"

START_TEST(test_LCH_Dict) {
  LCH_Dict *dict = LCH_DictCreate();
  ck_assert_ptr_nonnull(dict);

  const char keys[][LCH_BUFFER_SIZE] = {"one", "two",   "three", "four", "five",
                                        "six", "seven", "eight", "nine", "ten"};
  const size_t num_keys = LCH_LENGTH(keys);

  for (int i = 0; i < num_keys; i++) {
    int *data = (int *)malloc(sizeof(int));
    ck_assert_ptr_nonnull(data);
    *data = i;
    const LCH_Buffer *const key = LCH_BufferStaticFromString(keys[i]);
    ck_assert(LCH_DictSet(dict, key, data, free));
  }

  for (int i = 0; i < num_keys; i++) {
    const LCH_Buffer *const key = LCH_BufferStaticFromString(keys[i]);
    ck_assert(LCH_DictHasKey(dict, key));
    int *data = (int *)LCH_DictGet(dict, key);
    ck_assert_int_eq(*data, i);
  }

  const LCH_Buffer *const key = LCH_BufferStaticFromString("bogus");
  ck_assert(!LCH_DictHasKey(dict, key));

  LCH_DictDestroy(dict);
}
END_TEST

START_TEST(test_LCH_DictRemove) {
  LCH_Dict *dict = LCH_DictCreate();

  char buf[8];
  for (size_t i = 0; i < 100; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    ck_assert(LCH_DictSet(dict, key, strdup(buf), free));
  }

  for (size_t i = 0; i < 30; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    char *value = (char *)LCH_DictRemove(dict, key);
    ck_assert_str_eq(buf, value);
    free(value);
  }

  for (size_t i = 31; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    char *value = (char *)LCH_DictRemove(dict, key);
    ck_assert_str_eq(buf, value);
    free(value);
  }

  for (size_t i = 10; i < 20; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    ck_assert(LCH_DictSet(dict, key, strdup(buf), free));
  }

  for (size_t i = 0; i < 30; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    if (i >= 10 && i < 20) {
      ck_assert(LCH_DictHasKey(dict, key));
    } else {
      ck_assert(!LCH_DictHasKey(dict, key));
    }
  }

  for (size_t i = 30; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    ck_assert(LCH_DictHasKey(dict, key));
  }

  for (size_t i = 31; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    ck_assert(!LCH_DictHasKey(dict, key));
  }

  for (size_t i = 32; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    const LCH_Buffer *const key = LCH_BufferStaticFromString(buf);
    ck_assert(LCH_DictHasKey(dict, key));
  }

  LCH_DictDestroy(dict);
}
END_TEST

Suite *DictSuite(void) {
  Suite *s = suite_create("dict.c");
  {
    TCase *tc = tcase_create("LCH_Dict*");
    tcase_add_test(tc, test_LCH_Dict);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_DictRemove");
    tcase_add_test(tc, test_LCH_DictRemove);
    suite_add_tcase(s, tc);
  }
  return s;
}
