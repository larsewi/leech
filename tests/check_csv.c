#include <check.h>
#include <string.h>

#include "../lib/buffer.h"
#include "../lib/csv.h"
#include "../lib/definitions.h"
#include "../lib/leech.h"

START_TEST(test_LCH_CSVParseRecord) {
  LCH_List *record = LCH_CSVParseRecord("");
  ck_assert_ptr_nonnull(record);
  ck_assert_int_eq(LCH_ListLength(record), 1);
  LCH_ListDestroy(record);
}
END_TEST

START_TEST(test_LCH_ComposeCSV) {
  char *data[][LCH_BUFFER_SIZE] = {
      {(char *)"first name", (char *)"lastname", (char *)"born"},
      {(char *)"Paul", (char *)" McCar\ttney", (char *)" 1942 \t"},
      {(char *)"Ri\"ngo", (char *)"Starr", (char *)"1940"},
      {(char *)"John", (char *)"Lennon  ", (char *)"1940"},
      {(char *)"George", (char *)"Harr\r\nison", (char *)"1943"},
  };

  LCH_List *table = LCH_ListCreate();
  ck_assert_ptr_nonnull(table);

  for (size_t i = 0; i < 5; i++) {
    LCH_List *record = LCH_ListCreate();
    ck_assert_ptr_nonnull(record);

    for (size_t j = 0; j < 3; j++) {
      char *field = strdup(data[i][j]);
      ck_assert_ptr_nonnull(field);
      ck_assert(LCH_ListAppend(record, (void *)field, free));
    }

    ck_assert(LCH_ListAppend(table, record, (void (*)(void *))LCH_ListDestroy));
  }

  LCH_Buffer *buffer = NULL;
  ck_assert(LCH_CSVComposeTable(&buffer, table));
  ck_assert_ptr_nonnull(buffer);

  char *actual = LCH_BufferStringDup(buffer);
  LCH_BufferDestroy(buffer);
  ck_assert_ptr_nonnull(actual);

  char expected[] =
      "first name,lastname,born\r\n"
      "Paul,\" McCar\ttney\",\" 1942 \t\"\r\n"
      "\"Ri\"\"ngo\",Starr,1940\r\n"
      "John,\"Lennon  \",1940\r\n"
      "George,\"Harr\r\nison\",1943";

  ck_assert_str_eq(expected, actual);
  free(actual);
  LCH_ListDestroy(table);
}
END_TEST

START_TEST(test_LCH_ParseCSV) {
  char data[] =
      "first name,lastname,born\r\n"
      "Paul,\" McCar\ttney\",\" 1942 \t\"\r\n"
      "\"Ri\"\"ngo\",Starr,1940\r\n"
      "John,\"Lennon  \",1940\r\n"
      "George,\"Harr\r\nison\",1943";

  char *expected[][LCH_BUFFER_SIZE] = {
      {(char *)"first name", (char *)"lastname", (char *)"born"},
      {(char *)"Paul", (char *)" McCar\ttney", (char *)" 1942 \t"},
      {(char *)"Ri\"ngo", (char *)"Starr", (char *)"1940"},
      {(char *)"John", (char *)"Lennon  ", (char *)"1940"},
      {(char *)"George", (char *)"Harr\r\nison", (char *)"1943"},
  };

  LCH_List *table = LCH_CSVParseTable(data);
  ck_assert_ptr_nonnull(table);

  const size_t rows = LCH_ListLength(table);
  for (size_t row = 0; row < rows; row++) {
    LCH_List *record = (LCH_List *)LCH_ListGet(table, row);
    ck_assert_ptr_nonnull(record);

    const size_t cols = LCH_ListLength(record);
    for (size_t col = 0; col < cols; col++) {
      const char *const field = (char *)LCH_ListGet(record, col);
      ck_assert_ptr_nonnull(field);

      ck_assert_str_eq(expected[row][col], field);
    }
  }
  LCH_ListDestroy(table);
}
END_TEST

START_TEST(test_LCH_ParseCSVTrailingCRLF) {
  char data[] = "first name,lastname,born\r\n";

  char *expected[][LCH_BUFFER_SIZE] = {
      {(char *)"first name", (char *)"lastname", (char *)"born"}};

  LCH_List *table = LCH_CSVParseTable(data);
  ck_assert_ptr_nonnull(table);

  const size_t rows = LCH_ListLength(table);
  for (size_t row = 0; row < rows; row++) {
    LCH_List *record = (LCH_List *)LCH_ListGet(table, row);
    ck_assert_ptr_nonnull(record);

    const size_t cols = LCH_ListLength(record);
    for (size_t col = 0; col < cols; col++) {
      const char *const field = (char *)LCH_ListGet(record, col);
      ck_assert_ptr_nonnull(field);

      ck_assert_str_eq(expected[row][col], field);
    }
  }
  LCH_ListDestroy(table);
}
END_TEST

Suite *CSVSuite(void) {
  Suite *s = suite_create("csv.c");
  {
    TCase *tc = tcase_create("LCH_CSVParseRecord");
    tcase_add_test(tc, test_LCH_CSVParseRecord);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("Compose");
    tcase_add_test(tc, test_LCH_ComposeCSV);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("Parse");
    tcase_add_test(tc, test_LCH_ParseCSV);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("ParseTrailingCRLF");
    tcase_add_test(tc, test_LCH_ParseCSVTrailingCRLF);
    suite_add_tcase(s, tc);
  }
  return s;
}
