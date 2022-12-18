#include <check.h>

#include "../lib/definitions.h"
#include "../lib/leech.h"
#include "../lib/leech_csv.h"

START_TEST(test_CreateDestroyTable) {
  const char csv[] =
      "id,lastname,firstname,age\r\n"
      "0,McCartney,Paul,1942\r\n"
      "1,Starr,Ringo,1940\r\n"
      "2,Lennon,John,1940\r\n"
      "3,Harrison,George,1943\r\n";

  FILE *file = fopen("sample.csv", "w");
  ck_assert_ptr_nonnull(file);
  ck_assert_int_eq(fwrite(csv, 1, sizeof(csv), file), sizeof(csv));
  fclose(file);

  LCH_TableCreateInfo createInfo = {
      .primaryFields = "id",
      .subsidiaryFields = "lastname,firstname,age",
      .readLocator = "sample.csv",
      .writeLocator = "sample.csv",
      .readCallback = LCH_TableReadCallbackCSV,
      .writeCallback = LCH_TableWriteCallbackCSV,
  };

  LCH_Table *table = LCH_TableCreate(&createInfo);
  ck_assert_ptr_nonnull(table);
  ck_assert_int_eq(remove("sample.csv"), 0);
  LCH_TableDestroy(table);
}
END_TEST

Suite *TableSuite(void) {
  Suite *s = suite_create("table.c");
  {
    TCase *tc = tcase_create("Create/Destroy");
    tcase_add_test(tc, test_CreateDestroyTable);
    suite_add_tcase(s, tc);
  }
  return s;
}
