#include <check.h>

#include "../lib/definitions.h"
#include "../lib/leech.h"
#include "../lib/leech_csv.h"

START_TEST(test_LCH_TableReadCallbackCSV) {
  FILE *file = fopen("sample.csv", "w");
  ck_assert_ptr_nonnull(file);

  char content[] =
      "firstname,lastname,born\r\n"
      "Paul,McCartney,1942\r\n"
      "Ringo,Starr,1940\r\n"
      "John,Lennon,1940\r\n"
      "George,Harrison,1943\r\n";

  ck_assert_int_eq(fwrite(content, 1, sizeof(content), file), sizeof(content));
  fclose(file);

  char *expect[][LCH_BUFFER_SIZE] = {
      {(char *)"firstname", (char *)"lastname", (char *)"born"},
      {(char *)"Paul", (char *)"McCartney", (char *)"1942"},
      {(char *)"Ringo", (char *)"Starr", (char *)"1940"},
      {(char *)"John", (char *)"Lennon", (char *)"1940"},
      {(char *)"George", (char *)"Harrison", (char *)"1943"},
  };

  LCH_List *const table = LCH_TableReadCallbackCSV("sample.csv");
  ck_assert_ptr_nonnull(table);

  const size_t rows = LCH_ListLength(table);
  ck_assert_int_eq(rows, 5);

  for (size_t row = 0; row < rows; row++) {
    const LCH_List *const record = LCH_ListGet(table, row);
    ck_assert_ptr_nonnull(record);

    const size_t cols = LCH_ListLength(record);
    ck_assert_int_eq(cols, 3);

    for (size_t col = 0; col < cols; col++) {
      const char *const field = LCH_ListGet(record, col);
      ck_assert_ptr_nonnull(field);

      ck_assert_str_eq(field, expect[row][col]);
    }
  }

  ck_assert_int_eq(remove("sample.csv"), 0);
  LCH_ListDestroy(table);
}
END_TEST

Suite *LeechCSVSuite(void) {
  Suite *s = suite_create("leech_csv.c");
  {
    TCase *tc = tcase_create("LCH_TableReadCallbackCSV");
    tcase_add_test(tc, test_LCH_TableReadCallbackCSV);
    suite_add_tcase(s, tc);
  }
  return s;
}
