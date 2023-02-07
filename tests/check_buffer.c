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

START_TEST(test_LCH_BufferAllocate) {
  LCH_Buffer *buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_BufferPrintFormat(buffer, "first");
  size_t first_offset;
  ck_assert(LCH_BufferAllocate(buffer, sizeof(uint32_t), &first_offset));

  LCH_BufferPrintFormat(buffer, "second");
  size_t second_offset;
  ck_assert(LCH_BufferAllocate(buffer, sizeof(uint32_t), &second_offset));

  LCH_BufferPrintFormat(buffer, "end");

  const uint32_t second_value = 4321;
  LCH_BufferSet(buffer, second_offset, &second_value, sizeof(uint32_t));

  const uint32_t first_value = 1234;
  LCH_BufferSet(buffer, first_offset, &first_value, sizeof(uint32_t));

  uint32_t *first_actual = (uint32_t *)LCH_BufferGet(buffer, first_offset);
  ck_assert_int_eq(*first_actual, 1234);

  uint32_t *second_actual = (uint32_t *)LCH_BufferGet(buffer, second_offset);
  ck_assert_int_eq(*second_actual, 4321);

  LCH_BufferDestroy(buffer);
}
END_TEST

START_TEST(test_LCH_BufferAllocate2) {
  LCH_Buffer *buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);
  ck_assert_int_eq(LCH_BufferLength(buffer), 0);

  /****************************************************/

  size_t offset;
  ck_assert(LCH_BufferAllocate(buffer, sizeof(uint32_t), &offset));

  size_t before = LCH_BufferLength(buffer);
  ck_assert(LCH_BufferPrintFormat(buffer, "beatles"));
  size_t after = LCH_BufferLength(buffer);

  uint32_t length = htonl(after - before);
  LCH_BufferSet(buffer, offset, &length, sizeof(uint32_t));

  /****************************************************/

  ck_assert(LCH_BufferAllocate(buffer, sizeof(uint32_t), &offset));

  before = LCH_BufferLength(buffer);
  ck_assert(LCH_BufferPrintFormat(buffer, "pinkfloyd"));
  after = LCH_BufferLength(buffer);

  length = htonl(after - before);
  LCH_BufferSet(buffer, offset, &length, sizeof(uint32_t));

  /****************************************************/

  offset = 0;
  const uint32_t *len_ptr = LCH_BufferGet(buffer, offset);
  length = ntohl(*len_ptr);
  offset += sizeof(uint32_t);

  char *str = strndup(LCH_BufferGet(buffer, offset), length);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "beatles");
  offset += length;
  free(str);

  /****************************************************/

  len_ptr = LCH_BufferGet(buffer, offset);
  length = ntohl(*len_ptr);
  offset += sizeof(uint32_t);

  str = strndup(LCH_BufferGet(buffer, offset), length);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "pinkfloyd");
  offset += length;
  free(str);

  /****************************************************/

  LCH_BufferDestroy(buffer);
}
END_TEST

START_TEST(test_LCH_BufferHexBinDump) {
  LCH_Buffer *hex = LCH_BufferCreate();
  LCH_Buffer *bin = LCH_BufferCreate();

  const char *const hash = "b779abbc51c538027f9e8237db312e022891e9b7";

  ck_assert(LCH_BufferPrintFormat(hex, "%s", hash));
  ck_assert_str_eq(LCH_BufferGet(hex, 0), hash);

  ck_assert(LCH_BufferBinDump(bin, hex));
  LCH_BufferChop(hex, 0);
  ck_assert_str_eq(LCH_BufferGet(hex, 0), "");

  ck_assert(LCH_BufferHexDump(hex, bin));
  ck_assert_str_eq(LCH_BufferGet(hex, 0), hash);

  LCH_BufferDestroy(hex);
  LCH_BufferDestroy(bin);
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
    TCase *tc = tcase_create("LCH_BufferAllocate");
    tcase_add_test(tc, test_LCH_BufferAllocate);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_BufferAllocate2");
    tcase_add_test(tc, test_LCH_BufferAllocate2);
    suite_add_tcase(s, tc);
  }
  // {
  //   TCase *tc = tcase_create("LCH_BufferHexBinDump");
  //   tcase_add_test(tc, test_LCH_BufferHexBinDump);
  //   suite_add_tcase(s, tc);
  // }
  return s;
}
