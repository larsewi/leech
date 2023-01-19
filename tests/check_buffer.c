#include <check.h>
#include <stdio.h>

#include "../lib/buffer.h"
#include "../lib/leech.h"

START_TEST(test_LCH_Buffer) {
  LCH_Buffer *buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  for (int i = 0; i < 10; i++) {
    ck_assert(LCH_BufferPrintFormat(buffer, "Hello %s!\n", "buffer"));
  }

  char *actual = LCH_BufferStringDup(buffer);
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
  free(actual);
}
END_TEST

START_TEST(test_LCH_BufferAllocateLong) {
  LCH_Buffer *buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  uint32_t *l = LCH_BufferAllocateLong(buffer);
  *l = 1234;

  l = LCH_BufferAllocateLong(buffer);
  *l = 4321;

  uint32_t *actual = (uint32_t *) LCH_BufferGet(buffer);
  ck_assert_int_eq(*actual, 1234);

  actual += 1;
  ck_assert_int_eq(*actual, 4321);

  LCH_BufferDestroy(buffer);
}
END_TEST

Suite *BufferSuite(void) {
  Suite *s = suite_create("buffer.c");
  {
    TCase *tc = tcase_create("LCH_Buffer*");
    tcase_add_test(tc, test_LCH_Buffer);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_BufferAllocateLong");
    tcase_add_test(tc, test_LCH_BufferAllocateLong);
    suite_add_tcase(s, tc);
  }
  return s;
}
