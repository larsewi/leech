#include <check.h>
#include <stdlib.h>
#include <string.h>

#include "../leech/csv.h"
#include "../leech/debug_messenger.h"
#include "../leech/definitions.h"

START_TEST(test_LCH_ComposeCSV) {
  char *data[][LCH_BUFFER_SIZE] = {
      {"first name", "lastname", "born"}, {"Paul", " McCar\ttney", " 1942 \t"},
      {"Ri\"ngo", "Starr", "1940"},       {"John", "Lennon  ", "1940"},
      {"George", "Harr\r\nison", "1943"},
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

  LCH_Buffer *buffer = LCH_ComposeCSV(table);
  ck_assert_ptr_nonnull(buffer);

  char *actual = LCH_BufferGet(buffer);
  LCH_BufferDestroy(buffer);
  ck_assert_ptr_nonnull(actual);

  char expected[] =
      "first name,lastname,born\r\n"
      "Paul,\" McCar\ttney\",\" 1942 \t\"\r\n"
      "\"Ri\"\"ngo\",Starr,1940\r\n"
      "John,\"Lennon  \",1940\r\n"
      "George,\"Harr\r\nison\",1943";

  ck_assert_str_eq(expected, actual);
  LCH_ListDestroy(table);
}
END_TEST

Suite *CSVSuite(void) {
  Suite *s = suite_create("CSV");
  TCase *tc = tcase_create("Compose");
  tcase_add_test(tc, test_LCH_ComposeCSV);
  suite_add_tcase(s, tc);
  return s;
}
