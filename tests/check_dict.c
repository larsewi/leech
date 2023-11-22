#include <check.h>

#include "../lib/definitions.h"
#include "../lib/dict.h"
#include "../lib/leech.h"

START_TEST(test_LCH_Dict) {
  LCH_Dict *dict = LCH_DictCreate();
  ck_assert_ptr_nonnull(dict);

  const char keys[][LCH_BUFFER_SIZE] = {"one", "two",   "three", "four", "five",
                                        "six", "seven", "eight", "nine", "ten"};

  ck_assert_int_eq(LCH_DictLength(dict), 0);
  for (int i = 0; (size_t)i < LCH_LENGTH(keys); i++) {
    int *data = (int *)malloc(sizeof(int));
    ck_assert_ptr_nonnull(data);
    *data = i;
    ck_assert(LCH_DictSet(dict, keys[i], (void *)data, free));
  }
  ck_assert_int_eq(LCH_DictLength(dict), LCH_LENGTH(keys));

  for (int i = 0; (size_t)i < LCH_LENGTH(keys); i++) {
    ck_assert(LCH_DictHasKey(dict, keys[i]));
    int *data = (int *)LCH_DictGet(dict, keys[i]);
    ck_assert_int_eq(*data, i);
  }
  ck_assert(!LCH_DictHasKey(dict, "bogus"));
  ck_assert_int_eq(LCH_DictLength(dict), LCH_LENGTH(keys));

  LCH_DictDestroy(dict);
}
END_TEST

START_TEST(test_LCH_DictSetMinus) {
  LCH_Dict *left = LCH_DictCreate();
  LCH_DictSet(left, "one", strdup("1"), free);
  LCH_DictSet(left, "two", (void *)strdup("2"), free);
  LCH_DictSet(left, "three", (void *)strdup("3"), free);
  LCH_DictSet(left, "four", (void *)strdup("4"), free);
  LCH_DictSet(left, "five", (void *)strdup("5"), free);

  LCH_Dict *right = LCH_DictCreate();
  LCH_DictSet(right, "one", (void *)strdup("1"), free);
  LCH_DictSet(right, "three", (void *)strdup("3"), free);
  LCH_DictSet(right, "five", (void *)strdup("5"), free);

  LCH_Dict *result =
      LCH_DictSetMinus(left, right, (void *(*)(const void *))strdup, free);
  ck_assert(!LCH_DictHasKey(result, "one"));
  ck_assert(LCH_DictHasKey(result, "two"));
  ck_assert_str_eq((char *)LCH_DictGet(result, "two"), "2");
  ck_assert(!LCH_DictHasKey(result, "three"));
  ck_assert(LCH_DictHasKey(result, "four"));
  ck_assert_str_eq((char *)LCH_DictGet(result, "four"), "4");
  ck_assert(!LCH_DictHasKey(result, "five"));

  LCH_DictDestroy(left);
  LCH_DictDestroy(right);
  LCH_DictDestroy(result);
}
END_TEST

START_TEST(test_LCH_DictSetChangedIntersection) {
  LCH_Dict *left = LCH_DictCreate();
  LCH_DictSet(left, "one", (void *)strdup("foo"), free);
  LCH_DictSet(left, "two", (void *)strdup("2"), free);
  LCH_DictSet(left, "three", (void *)strdup("3"), free);
  LCH_DictSet(left, "four", (void *)strdup("4"), free);
  LCH_DictSet(left, "five", (void *)strdup("bar"), free);

  LCH_Dict *right = LCH_DictCreate();
  LCH_DictSet(right, "one", (void *)strdup("1"), free);
  LCH_DictSet(right, "three", (void *)strdup("3"), free);
  LCH_DictSet(right, "five", (void *)strdup("5"), free);

  LCH_Dict *result = LCH_DictSetChangedIntersection(
      left, right, (void *(*)(const void *))strdup, free,
      (int (*)(const void *, const void *))strcmp);
  ck_assert(LCH_DictHasKey(result, "one"));
  ck_assert_str_eq((char *)LCH_DictGet(result, "one"), "foo");
  ck_assert(!LCH_DictHasKey(result, "two"));
  ck_assert(!LCH_DictHasKey(result, "three"));
  ck_assert(!LCH_DictHasKey(result, "four"));
  ck_assert(LCH_DictHasKey(result, "five"));
  ck_assert_str_eq((char *)LCH_DictGet(result, "five"), "bar");

  LCH_DictDestroy(left);
  LCH_DictDestroy(right);
  LCH_DictDestroy(result);
}
END_TEST

START_TEST(test_LCH_DictRemove) {
  LCH_Dict *dict = LCH_DictCreate();

  char buf[8];
  for (size_t i = 0; i < 100; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    ck_assert(LCH_DictSet(dict, buf, strdup(buf), free));
  }

  for (size_t i = 0; i < 30; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    char *value = (char *)LCH_DictRemove(dict, buf);
    ck_assert_str_eq(buf, value);
    free(value);
  }

  for (size_t i = 31; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    char *value = (char *)LCH_DictRemove(dict, buf);
    ck_assert_str_eq(buf, value);
    free(value);
  }

  for (size_t i = 10; i < 20; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    ck_assert(LCH_DictSet(dict, buf, strdup(buf), free));
  }

  for (size_t i = 0; i < 30; i++) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    if (i >= 10 && i < 20) {
      ck_assert(LCH_DictHasKey(dict, buf));
    } else {
      ck_assert(!LCH_DictHasKey(dict, buf));
    }
  }

  for (size_t i = 30; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    ck_assert(LCH_DictHasKey(dict, buf));
  }

  for (size_t i = 31; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    ck_assert(!LCH_DictHasKey(dict, buf));
  }

  for (size_t i = 32; i < 100; i += 5) {
    ck_assert_int_lt(snprintf(buf, sizeof(buf), "%zu", i), sizeof(buf));
    ck_assert(LCH_DictHasKey(dict, buf));
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
    TCase *tc = tcase_create("LCH_DictSetMinus");
    tcase_add_test(tc, test_LCH_DictSetMinus);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_DictSetChangedIntersection");
    tcase_add_test(tc, test_LCH_DictSetChangedIntersection);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_DictRemove");
    tcase_add_test(tc, test_LCH_DictRemove);
    suite_add_tcase(s, tc);
  }
  return s;
}
