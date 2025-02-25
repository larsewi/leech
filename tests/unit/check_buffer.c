#include <check.h>

#ifdef _WIN32
#include <winsock2.h>
#else  // _WIN32
#include <arpa/inet.h>
#endif  // _WIN32

#include "../lib/buffer.h"
#include "../lib/string_lib.h"

START_TEST(test_LCH_Buffer) {
  LCH_Buffer *buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  for (int i = 0; i < 10; i++) {
    ck_assert(LCH_BufferPrintFormat(buffer, "Hello %s!\n", "buffer"));
  }

  char *actual = LCH_BufferToString(buffer);
  ck_assert_ptr_nonnull(buffer);

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

START_TEST(test_LCH_BufferTrim) {
  {
    LCH_Buffer *const buffer = LCH_BufferFromString("leech");
    LCH_BufferTrim(buffer, ' ');
    const char *const actual = LCH_BufferData(buffer);
    ck_assert_str_eq(actual, "leech");
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_BufferFromString("  leech");
    LCH_BufferTrim(buffer, ' ');
    const char *const actual = LCH_BufferData(buffer);
    ck_assert_str_eq(actual, "leech");
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_BufferFromString("leech  ");
    LCH_BufferTrim(buffer, ' ');
    const char *const actual = LCH_BufferData(buffer);
    ck_assert_str_eq(actual, "leech");
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_BufferFromString("  leech  ");
    LCH_BufferTrim(buffer, ' ');
    const char *const actual = LCH_BufferData(buffer);
    ck_assert_str_eq(actual, "leech");
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_BufferFromString("  ");
    LCH_BufferTrim(buffer, ' ');
    const char *const actual = LCH_BufferData(buffer);
    ck_assert_str_eq(actual, "");
    LCH_BufferDestroy(buffer);
  }
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

  uint32_t *first_actual = (uint32_t *)(LCH_BufferData(buffer) + first_offset);
  ck_assert_int_eq(*first_actual, 1234);

  uint32_t *second_actual =
      (uint32_t *)(LCH_BufferData(buffer) + second_offset);
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
  const uint32_t *len_ptr = (uint32_t *)(LCH_BufferData(buffer) + offset);
  length = ntohl(*len_ptr);
  offset += sizeof(uint32_t);

  char *str = LCH_StringNDuplicate(LCH_BufferData(buffer) + offset, length);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "beatles");
  offset += length;
  free(str);

  /****************************************************/

  len_ptr = (const uint32_t *)(LCH_BufferData(buffer) + offset);
  length = ntohl(*len_ptr);
  offset += sizeof(uint32_t);

  str = LCH_StringNDuplicate(LCH_BufferData(buffer) + offset, length);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "pinkfloyd");
  offset += length;
  free(str);

  /****************************************************/

  LCH_BufferDestroy(buffer);
}
END_TEST

START_TEST(test_LCH_BufferBytesToHex) {
  const char data[] = {'\x01', '\x23', '\x45', '\x67',
                       '\x89', '\xab', '\xcd', '\xef'};

  LCH_Buffer *bytes = LCH_BufferCreate();
  ck_assert_ptr_nonnull(bytes);

  for (size_t i = 0; i < sizeof(data); i++) {
    ck_assert(LCH_BufferAppend(bytes, data[i]));
  }

  LCH_Buffer *hex = LCH_BufferCreate();
  ck_assert_ptr_nonnull(hex);

  ck_assert(LCH_BufferBytesToHex(hex, bytes));
  LCH_BufferDestroy(bytes);

  char *str = LCH_BufferToString(hex);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "0123456789abcdef");
  free(str);
}
END_TEST

START_TEST(test_LCH_BufferIsPrintable) {
  {
    LCH_Buffer *buffer = LCH_BufferFromString("leech");
    LCH_BufferAppend(buffer, 0);
    ck_assert(!LCH_BufferIsPrintable(buffer));
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *buffer = LCH_BufferFromString("leech");
    LCH_BufferAppend(buffer, 31);
    ck_assert(!LCH_BufferIsPrintable(buffer));
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *buffer = LCH_BufferFromString("leech");
    LCH_BufferAppend(buffer, 32);
    ck_assert(LCH_BufferIsPrintable(buffer));
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *buffer = LCH_BufferFromString("leech");
    LCH_BufferAppend(buffer, 126);
    ck_assert(LCH_BufferIsPrintable(buffer));
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *buffer = LCH_BufferFromString("leech");
    LCH_BufferAppend(buffer, 127);
    ck_assert(!LCH_BufferIsPrintable(buffer));
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *buffer = LCH_BufferFromString("leech");
    LCH_BufferAppend(buffer, 255);
    ck_assert(!LCH_BufferIsPrintable(buffer));
    LCH_BufferDestroy(buffer);
  }
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
    TCase *tc = tcase_create("LCH_BufferTrim");
    tcase_add_test(tc, test_LCH_BufferTrim);
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
  {
    TCase *tc = tcase_create("LCH_BufferBytesToHex");
    tcase_add_test(tc, test_LCH_BufferBytesToHex);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_BufferIsPrintable");
    tcase_add_test(tc, test_LCH_BufferIsPrintable);
    suite_add_tcase(s, tc);
  }
  return s;
}
