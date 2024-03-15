#include <check.h>
#include <string.h>
#include <unistd.h>

#include "../lib/buffer.h"
#include "../lib/csv.h"
#include "../lib/utils.h"

START_TEST(test_LCH_CSVParseField) {
  {  // Simple field
    const char *const csv = "leech";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Empy field
    const char *const csv = "";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Empty field / multiple columns
    const char *const csv = ",";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Only spaces
    const char *const csv = "      ";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Multiple fields
    const char *const csv = "leech,1.0.0";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Multiple columns
    const char *const csv = "leech\r\n1.0.0";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Leading spaces
    const char *const csv = "  leech";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Trailing spaces
    const char *const csv = "leech  ";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Escaped
    const char *const csv = "\"leech\"";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Escaped empty
    const char *const csv = "\"\"";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Escaped multiple fields
    const char *const csv = "\"leech\",1.0.0";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Escaped multiple columns
    const char *const csv = "\"leech\"\r\n1.0.0";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Escaped leading spaces
    const char *const csv = "  \"leech\"";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Escaped trailing spaces
    const char *const csv = "\"leech\"  ";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = "leech";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Escaped inner spaces
    const char *const csv = "\" leech \"";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = " leech ";
    const size_t expected_len = strlen(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_str_eq(actual, expected);
    LCH_BufferDestroy(field);
  }
  {  // Binary (must always be escaped)
    const char csv[] = {'"', 0x01, 0x00, 0x01, '"'};
    LCH_Buffer *const field = LCH_CSVParseField(csv, sizeof(csv));
    ck_assert_ptr_nonnull(field);

    const char *const actual = (const char *)LCH_BufferData(field);
    const size_t actual_len = LCH_BufferLength(field);

    const char expected[] = {0x01, 0x00, 0x01};
    const size_t expected_len = sizeof(expected);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_mem_eq(actual, expected, sizeof(expected));
    LCH_BufferDestroy(field);
  }
  {  // Missing terminating DQUOTE
    const char *const csv = "\" leech ";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_null(field);
  }
  {  // Non-escaped non-TEXTDATA
    const char *const csv = " leech \"";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_null(field);
  }
  { /* Field is not terminated by End-of-Buffer, End-of-Field (i.e., COMMA) or
     * End-of-Record (i.e., CRLF) */
    const char *const csv = "\"leech\"a";
    LCH_Buffer *const field = LCH_CSVParseField(csv, strlen(csv));
    ck_assert_ptr_null(field);
  }
}
END_TEST

START_TEST(test_LCH_CSVParseRecord) {
  {  // Empty
    const char csv[] = "";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_fields, 1);

    LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, 0);
    const char *const actual = LCH_BufferData(field);
    ck_assert_str_eq(actual, "");

    LCH_ListDestroy(record);
  }
  {  // Single field
    const char csv[] = "leech";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_fields, 1);

    LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, 0);
    const char *const actual = LCH_BufferData(field);
    ck_assert_str_eq(actual, "leech");

    LCH_ListDestroy(record);
  }
  {  // Multiple fields
    const char csv[] = "leech,1.0.0";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_fields, 2);

    const char *const expected[] = {"leech", "1.0.0"};
    for (size_t i = 0; i < num_fields; i++) {
      LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, i);
      const char *const actual = LCH_BufferData(field);
      ck_assert_str_eq(actual, expected[i]);
    }

    LCH_ListDestroy(record);
  }
  {  // Multiple rows
    const char csv[] = "leech,1.0.0\r\nbogus,1.0.1";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_fields, 2);

    const char *const expected[] = {"leech", "1.0.0"};
    for (size_t i = 0; i < num_fields; i++) {
      LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, i);
      const char *const actual = LCH_BufferData(field);
      ck_assert_str_eq(actual, expected[i]);
    }

    LCH_ListDestroy(record);
  }
  {  // Multiple empty fields
    const char csv[] = ",,\r\n";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_fields, 3);

    const char *const expected[] = {"", "", ""};
    for (size_t i = 0; i < num_fields; i++) {
      LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, i);
      const char *const actual = LCH_BufferData(field);
      ck_assert_str_eq(actual, expected[i]);
    }

    LCH_ListDestroy(record);
  }
  {  // Missing terminating DQUOTE
    const char csv[] = "leech,\"1.0.0\r\nbogus,1.0.1";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_null(record);
  }
  {  // Non-escaped non-TEXTDATA
    const char csv[] = "leech,1.0\".0\r\nbogus,1.0.1";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_null(record);
  }
  { /* Field is not terminated by End-of-Buffer, End-of-Field (i.e., COMMA) or
     * End-of-Record (i.e., CRLF) */
    const char csv[] = "leech,\"1.0.0\"a\r\nbogus,1.0.1";
    LCH_List *const record = LCH_CSVParseRecord(csv, strlen(csv));
    ck_assert_ptr_null(record);
  }
}
END_TEST

START_TEST(test_LCH_CSVParseTable) {
  {  // Empty CSV
    const char *const csv = "";

    LCH_List *const table = LCH_CSVParseTable(csv, strlen(csv));
    ck_assert_ptr_nonnull(table);

    const size_t num_records = LCH_ListLength(table);
    ck_assert_int_eq(num_records, 1);

    const LCH_List *const record = (LCH_List *)LCH_ListGet(table, 0);
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_records, 1);

    const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, 0);
    ck_assert_ptr_nonnull(field);

    const char *const actual = LCH_BufferData(field);
    ck_assert_str_eq(actual, "");

    LCH_ListDestroy(table);
  }
  {  // Empty CSV trailing CRLF
    const char *const csv = "\r\n";

    LCH_List *const table = LCH_CSVParseTable(csv, strlen(csv));
    ck_assert_ptr_nonnull(table);

    const size_t num_records = LCH_ListLength(table);
    ck_assert_int_eq(num_records, 1);

    const LCH_List *const record = (LCH_List *)LCH_ListGet(table, 0);
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_fields, 1);

    const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, 0);
    ck_assert_ptr_nonnull(field);

    const char *const actual = LCH_BufferData(field);
    ck_assert_str_eq(actual, "");

    LCH_ListDestroy(table);
  }
  {  // Two empty rows
    const char *const csv = "\r\n\r\n";

    LCH_List *const table = LCH_CSVParseTable(csv, strlen(csv));
    ck_assert_ptr_nonnull(table);

    const size_t num_records = LCH_ListLength(table);
    ck_assert_int_eq(num_records, 2);

    for (size_t i = 0; i < num_records; i++) {
      const LCH_List *const record = (LCH_List *)LCH_ListGet(table, i);
      ck_assert_ptr_nonnull(record);

      const size_t num_fields = LCH_ListLength(record);
      ck_assert_int_eq(num_fields, 1);

      const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, 0);
      ck_assert_ptr_nonnull(field);

      const char *const actual = LCH_BufferData(field);
      ck_assert_str_eq(actual, "");
    }

    LCH_ListDestroy(table);
  }
  {  // Two empty rows / two empty columns
    const char *const csv = ",\r\n,";

    LCH_List *const table = LCH_CSVParseTable(csv, strlen(csv));
    ck_assert_ptr_nonnull(table);

    const size_t num_records = LCH_ListLength(table);
    ck_assert_int_eq(num_records, 2);

    for (size_t i = 0; i < num_records; i++) {
      const LCH_List *const record = (LCH_List *)LCH_ListGet(table, i);
      ck_assert_ptr_nonnull(record);

      const size_t num_fields = LCH_ListLength(record);
      ck_assert_int_eq(num_fields, 2);

      for (size_t j = 0; j < num_fields; j++) {
        const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, j);
        ck_assert_ptr_nonnull(field);

        const char *const actual = LCH_BufferData(field);
        ck_assert_str_eq(actual, "");
      }
    }

    LCH_ListDestroy(table);
  }
  {  // Multiple rows / multiple columns
    const char *const csv =
        "first name,  last name,  born\r\n"
        "Paul,        McCartney,  1942\r\n"
        "Ringo,       Starr,      1940\r\n"
        "John,        Lennon,     1940\r\n"
        "George,      Harrison,   1943\r\n";

    LCH_List *const table = LCH_CSVParseTable(csv, strlen(csv));
    ck_assert_ptr_nonnull(table);

    const size_t num_records = LCH_ListLength(table);
    ck_assert_int_eq(num_records, 5);

    const char *const expected[][3] = {
        {"first name", "last name", "born"}, {"Paul", "McCartney", "1942"},
        {"Ringo", "Starr", "1940"},          {"John", "Lennon", "1940"},
        {"George", "Harrison", "1943"},
    };

    for (size_t i = 0; i < num_records; i++) {
      const LCH_List *const record = (LCH_List *)LCH_ListGet(table, i);
      ck_assert_ptr_nonnull(record);

      const size_t num_fields = LCH_ListLength(record);
      ck_assert_int_eq(num_fields, 3);

      for (size_t j = 0; j < num_fields; j++) {
        const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, j);
        const char *const actual = LCH_BufferData(field);
        ck_assert_str_eq(actual, expected[i][j]);
      }
    }

    LCH_ListDestroy(table);
  }
  {  // Binary test
    LCH_Buffer *const buffer = LCH_BufferCreate();
    ck_assert_ptr_nonnull(buffer);

    char ch = 0;
    for (size_t i = 0; i < 8; i++) {
      if (i != 0) {
        ck_assert(LCH_BufferAppend(buffer, '\r'));
        ck_assert(LCH_BufferAppend(buffer, '\n'));
      }
      for (size_t j = 0; j < 8; j++) {
        if (j != 0) {
          ck_assert(LCH_BufferAppend(buffer, ','));
        }
        ck_assert(LCH_BufferAppend(buffer, '"'));
        for (size_t k = 0; k < 4; k++) {
          if (ch == '"') {
            // Escape quote
            ck_assert(LCH_BufferAppend(buffer, '"'));
          }
          ck_assert(LCH_BufferAppend(buffer, ch++));
        }
        ck_assert(LCH_BufferAppend(buffer, '"'));
      }
    }

    const char *const csv = LCH_BufferData(buffer);
    const size_t length = LCH_BufferLength(buffer);

    LCH_List *const table = LCH_CSVParseTable(csv, length);
    ck_assert_ptr_nonnull(table);

    LCH_BufferDestroy(buffer);

    const size_t num_records = LCH_ListLength(table);
    ck_assert_int_eq(num_records, 8);

    char expected = 0;
    for (size_t i = 0; i < num_records; i++) {
      const LCH_List *const record = (LCH_List *)LCH_ListGet(table, i);
      ck_assert_ptr_nonnull(record);

      const size_t num_fields = LCH_ListLength(record);
      ck_assert_int_eq(num_fields, 8);

      for (size_t j = 0; j < num_fields; j++) {
        const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, j);
        ck_assert_ptr_nonnull(field);

        const size_t num_bytes = LCH_BufferLength(field);
        ck_assert_int_eq(num_bytes, 4);

        for (size_t k = 0; k < num_bytes; k++) {
          const char actual = LCH_BufferData(field)[k];
          ck_assert_int_eq(actual, expected++);
        }
      }
    }

    LCH_ListDestroy(table);
  }
}
END_TEST

START_TEST(test_LCH_CSVParseFile) {
  char filename[] = "test_LCH_CSVParseFile_XXXXXX";
  ck_assert_str_ne(mktemp(filename), "");

  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  char ch = 0;
  for (size_t i = 0; i < 8; i++) {
    if (i != 0) {
      ck_assert(LCH_BufferAppend(buffer, '\r'));
      ck_assert(LCH_BufferAppend(buffer, '\n'));
    }
    for (size_t j = 0; j < 8; j++) {
      if (j != 0) {
        ck_assert(LCH_BufferAppend(buffer, ','));
      }
      ck_assert(LCH_BufferAppend(buffer, '"'));
      for (size_t k = 0; k < 4; k++) {
        if (ch == '"') {
          // Escape quote
          ck_assert(LCH_BufferAppend(buffer, '"'));
        }
        ck_assert(LCH_BufferAppend(buffer, ch++));
      }
      ck_assert(LCH_BufferAppend(buffer, '"'));
    }
  }

  ck_assert(LCH_BufferWriteFile(buffer, filename));

  LCH_List *const table = LCH_CSVParseFile(filename);
  ck_assert_ptr_nonnull(table);

  LCH_BufferDestroy(buffer);

  const size_t num_records = LCH_ListLength(table);
  ck_assert_int_eq(num_records, 8);

  char expected = 0;
  for (size_t i = 0; i < num_records; i++) {
    const LCH_List *const record = (LCH_List *)LCH_ListGet(table, i);
    ck_assert_ptr_nonnull(record);

    const size_t num_fields = LCH_ListLength(record);
    ck_assert_int_eq(num_fields, 8);

    for (size_t j = 0; j < num_fields; j++) {
      const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, j);
      ck_assert_ptr_nonnull(field);

      const size_t num_bytes = LCH_BufferLength(field);
      ck_assert_int_eq(num_bytes, 4);

      for (size_t k = 0; k < num_bytes; k++) {
        const char actual = LCH_BufferData(field)[k];
        ck_assert_int_eq(actual, expected++);
      }
    }
  }

  LCH_ListDestroy(table);
  unlink(filename);
}
END_TEST

START_TEST(test_LCH_CSVComposeField) {
  {  // Simple allocate buffer
    const char *const data = "leech";
    const size_t length = strlen(data);

    LCH_Buffer *csv = NULL;
    ck_assert(LCH_CSVComposeField(&csv, data, length));
    ck_assert_ptr_nonnull(csv);

    const size_t actual_len = LCH_BufferLength(csv);
    ck_assert_int_eq(actual_len, 5);

    const char *const actual = LCH_BufferData(csv);
    ck_assert_str_eq(actual, "leech");

    LCH_BufferDestroy(csv);
  }
  {  // Append to existing buffer
    LCH_Buffer *csv = LCH_BufferFromString("leech,");
    ck_assert_ptr_nonnull(csv);

    const char *const data = "1.0.0";
    const size_t length = strlen(data);

    ck_assert(LCH_CSVComposeField(&csv, data, length));

    const size_t actual_len = LCH_BufferLength(csv);
    ck_assert_int_eq(actual_len, 11);

    const char *const actual = LCH_BufferData(csv);
    ck_assert_str_eq(actual, "leech,1.0.0");

    LCH_BufferDestroy(csv);
  }
  {  // Non-TEXTDATA
    const char *const data = "leech,1.0.0";
    const size_t length = strlen(data);

    LCH_Buffer *csv = NULL;
    ck_assert(LCH_CSVComposeField(&csv, data, length));
    ck_assert_ptr_nonnull(csv);

    const size_t actual_len = LCH_BufferLength(csv);
    ck_assert_int_eq(actual_len, 13);

    const char *const actual = LCH_BufferData(csv);
    ck_assert_str_eq(actual, "\"leech,1.0.0\"");

    LCH_BufferDestroy(csv);
  }
  {  // Non-TEXTDATA
    const char data[] = {'\x00', '\xFF', '\x00', '\xFF'};
    const size_t length = sizeof(data);

    LCH_Buffer *csv = NULL;
    ck_assert(LCH_CSVComposeField(&csv, data, length));
    ck_assert_ptr_nonnull(csv);

    const char expected[] = {'"', '\x00', '\xFF', '\x00', '\xFF', '"'};
    size_t expected_len = sizeof(expected);

    const char *const actual = LCH_BufferData(csv);
    const size_t actual_len = LCH_BufferLength(csv);

    ck_assert_int_eq(actual_len, expected_len);
    ck_assert_mem_eq(actual, expected, actual_len);

    LCH_BufferDestroy(csv);
  }
}
END_TEST

START_TEST(test_LCH_CSVComposeRecord) {
  const char *data[] = {"leech", "1.0.0"};

  LCH_List *const record = LCH_ListCreate();
  ck_assert_ptr_nonnull(record);

  for (size_t i = 0; i < sizeof(data) / sizeof(*data); i++) {
    LCH_Buffer *const field = LCH_BufferFromString(data[i]);
    ck_assert_ptr_nonnull(field);
    ck_assert(LCH_ListAppend(record, field, LCH_BufferDestroy));
  }

  LCH_Buffer *csv = NULL;
  ck_assert(LCH_CSVComposeRecord(&csv, record));
  ck_assert_ptr_nonnull(csv);
  LCH_ListDestroy(record);

  const char *const expected = "leech,1.0.0";
  const char *const actual = LCH_BufferData(csv);
  ck_assert_str_eq(actual, expected);

  LCH_BufferDestroy(csv);
}
END_TEST

START_TEST(test_LCH_CSVComposeTable) {
  const char *const data[][3] = {
      {"first name", "last name", "born"}, {"Paul", "McCartney", "1942"},
      {"Ringo", "Starr", "1940"},          {"John", "Lennon", "1940"},
      {"George", "Harrison", "1943"},
  };

  LCH_List *const table = LCH_ListCreate();
  ck_assert_ptr_nonnull(table);
  for (size_t i = 0; i < sizeof(data) / sizeof(*data); i++) {
    LCH_List *const record = LCH_ListCreate();
    ck_assert_ptr_nonnull(record);
    for (size_t j = 0; j < sizeof(*data) / sizeof(**data); j++) {
      LCH_Buffer *const field = LCH_BufferFromString(data[i][j]);
      ck_assert_ptr_nonnull(field);
      ck_assert(LCH_ListAppend(record, field, LCH_BufferDestroy));
    }
    ck_assert(LCH_ListAppend(table, record, LCH_ListDestroy));
  }

  LCH_Buffer *csv = NULL;
  ck_assert(LCH_CSVComposeTable(&csv, table));
  LCH_ListDestroy(table);
  ck_assert_ptr_nonnull(csv);

  const char *const expected =
      "first name,last name,born\r\n"
      "Paul,McCartney,1942\r\n"
      "Ringo,Starr,1940\r\n"
      "John,Lennon,1940\r\n"
      "George,Harrison,1943";
  const char *const actual = LCH_BufferData(csv);
  ck_assert_str_eq(actual, expected);

  LCH_BufferDestroy(csv);
}
END_TEST

START_TEST(test_LCH_CSVComposeFile) {
  char filename[] = "test_LCH_CSVParseFile_XXXXXX";
  // TODO: The use of `mktemp' is dangerous, better use `mkstemp' or `mkdtemp'
  ck_assert_str_ne(mktemp(filename), "");

  LCH_List *const table = LCH_ListCreate();
  ck_assert_ptr_nonnull(table);

  char ch = 0;
  for (size_t i = 0; i < 8; i++) {
    LCH_List *const record = LCH_ListCreate();
    ck_assert_ptr_nonnull(record);
    for (size_t j = 0; j < 8; j++) {
      LCH_Buffer *const field = LCH_BufferCreate();
      ck_assert_ptr_nonnull(field);
      for (size_t k = 0; k < 4; k++) {
        ck_assert(LCH_BufferAppend(field, ch++));
      }
      ck_assert(LCH_ListAppend(record, field, LCH_BufferDestroy));
    }
    ck_assert(LCH_ListAppend(table, record, LCH_ListDestroy));
  }

  ck_assert(LCH_CSVComposeFile(table, filename));
  LCH_ListDestroy(table);

  const char *const expected =
      "\"\x00\x01\x02\x03\",\"\x04\x05\x06\x07\","
      "\"\x08\x09\x0a\x0b\",\"\x0c\x0d\x0e\x0f\","
      "\"\x10\x11\x12\x13\",\"\x14\x15\x16\x17\","
      "\"\x18\x19\x1a\x1b\",\"\x1c\x1d\x1e\x1f\"\r\n"
      "\" !\"\"#\",$\x25&',()*+,\",-./\",0123,4567,89:;,<=>?\r\n"
      "@ABC,DEFG,HIJK,LMNO,PQRS,TUVW,XYZ[,\\]^_\r\n"
      "`abc,defg,hijk,lmno,pqrs,tuvw,xyz{,\"|}~\x7f\"\r\n"
      "\"\x80\x81\x82\x83\",\"\x84\x85\x86\x87\","
      "\"\x88\x89\x8a\x8b\",\"\x8c\x8d\x8e\x8f\","
      "\"\x90\x91\x92\x93\",\"\x94\x95\x96\x97\","
      "\"\x98\x99\x9a\x9b\",\"\x9c\x9d\x9e\x9f\"\r\n"
      "\"\xa0\xa1\xa2\xa3\",\"\xa4\xa5\xa6\xa7\","
      "\"\xa8\xa9\xaa\xab\",\"\xac\xad\xae\xaf\","
      "\"\xb0\xb1\xb2\xb3\",\"\xb4\xb5\xb6\xb7\","
      "\"\xb8\xb9\xba\xbb\",\"\xbc\xbd\xbe\xbf\"\r\n"
      "\"\xc0\xc1\xc2\xc3\",\"\xc4\xc5\xc6\xc7\","
      "\"\xc8\xc9\xca\xcb\",\"\xcc\xcd\xce\xcf\","
      "\"\xd0\xd1\xd2\xd3\",\"\xd4\xd5\xd6\xd7\","
      "\"\xd8\xd9\xda\xdb\",\"\xdc\xdd\xde\xdf\"\r\n"
      "\"\xe0\xe1\xe2\xe3\",\"\xe4\xe5\xe6\xe7\","
      "\"\xe8\xe9\xea\xeb\",\"\xec\xed\xee\xef\","
      "\"\xf0\xf1\xf2\xf3\",\"\xf4\xf5\xf6\xf7\","
      "\"\xf8\xf9\xfa\xfb\",\"\xfc\xfd\xfe\xff\"";

  size_t actual_size;
  char *const actual = LCH_FileRead(filename, &actual_size);
  ck_assert_ptr_nonnull(actual);
  ck_assert_mem_eq(actual, expected, actual_size);
  free(actual);
  unlink(filename);
}
END_TEST

Suite *CSVSuite(void) {
  Suite *s = suite_create("csv.c");
  {
    TCase *tc = tcase_create("LCH_CSVParseField");
    tcase_add_test(tc, test_LCH_CSVParseField);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_CSVParseRecord");
    tcase_add_test(tc, test_LCH_CSVParseRecord);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_CSVParseTable");
    tcase_add_test(tc, test_LCH_CSVParseTable);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_CSVParseFile");
    tcase_add_test(tc, test_LCH_CSVParseFile);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_CSVComposeField");
    tcase_add_test(tc, test_LCH_CSVComposeField);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_CSVComposeRecord");
    tcase_add_test(tc, test_LCH_CSVComposeRecord);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_CSVComposeTable");
    tcase_add_test(tc, test_LCH_CSVComposeTable);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_CSVComposeFile");
    tcase_add_test(tc, test_LCH_CSVComposeFile);
    suite_add_tcase(s, tc);
  }
  return s;
}
