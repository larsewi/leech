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
  ck_assert_str_eq(LCH_DictGet(result, "two"), "2");
  ck_assert(!LCH_DictHasKey(result, "three"));
  ck_assert(LCH_DictHasKey(result, "four"));
  ck_assert_str_eq(LCH_DictGet(result, "four"), "4");
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
  ck_assert_str_eq(LCH_DictGet(result, "one"), "foo");
  ck_assert(!LCH_DictHasKey(result, "two"));
  ck_assert(!LCH_DictHasKey(result, "three"));
  ck_assert(!LCH_DictHasKey(result, "four"));
  ck_assert(LCH_DictHasKey(result, "five"));
  ck_assert_str_eq(LCH_DictGet(result, "five"), "bar");

  LCH_DictDestroy(left);
  LCH_DictDestroy(right);
  LCH_DictDestroy(result);
}
END_TEST

START_TEST(test_LCH_DictIter) {
  char *stuff[] = {(char *)"one", (char *)"two", (char *)"three",
                   (char *)"four", (char *)"five"};

  LCH_Dict *dict = LCH_DictCreate();

  for (size_t i = 0; i < 5; i++) {
    LCH_DictSet(dict, stuff[i], strdup(stuff[i]), free);
  }

  LCH_DictIter *iter = LCH_DictIterCreate(dict);
  ck_assert_ptr_nonnull(iter);

  size_t i = 0;
  while (LCH_DictIterHasNext(iter)) {
    char *key = LCH_DictIterGetKey(iter);
    char *val = (char *)LCH_DictIterGetValue(iter);
    ck_assert_str_eq(key, val);
    i += 1;
  }

  free(iter);
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
    TCase *tc = tcase_create("LCH_DictIter*");
    tcase_add_test(tc, test_LCH_DictIter);
    suite_add_tcase(s, tc);
  }
  return s;
}
