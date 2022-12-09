#include <check.h>

#include "../leech/debug_messenger.h"
#include "../leech/definitions.h"
#include "../leech/dict.h"

START_TEST(test_LCH_Dict) {
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

Suite *DictSuite(void) {
  Suite *s = suite_create("dict.c");
  {
    TCase *tc = tcase_create("LCH_Dict*");
    tcase_add_test(tc, test_LCH_Dict);
    suite_add_tcase(s, tc);
  }
  return s;
}
