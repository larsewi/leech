#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lib/leech.h"

START_TEST(test_LCH_Buffer) {
  LCH_Buffer *buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  for (int i = 0; i < 10; i++) {
    ck_assert(LCH_BufferAppend(buffer, "Hello %s!\n", "buffer"));
  }

  char *actual = LCH_BufferGet(buffer);
  ck_assert_ptr_nonnull(buffer);

  LCH_BufferDestroy(buffer);

  char exptected[] = {
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"
      "Hello buffer!\n"};

  ck_assert_str_eq(actual, exptected);
}
END_TEST

Suite *BufferSuite(void) {
  Suite *s = suite_create("buffer.c");
  TCase *tc = tcase_create("LCH_Buffer*");
  tcase_add_test(tc, test_LCH_Buffer);
  suite_add_tcase(s, tc);
  return s;
}
