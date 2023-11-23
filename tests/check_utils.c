#include <check.h>
#include <limits.h>

#include "../lib/definitions.h"
#include "../lib/dict.h"
#include "../lib/leech.h"
#include "../lib/utils.c"

START_TEST(test_ExtractFieldsAtIndices) {
  LCH_List *header = LCH_CSVParseRecord("one,two,three,four,five");
  ck_assert_ptr_nonnull(header);
  ck_assert_int_eq(LCH_ListLength(header), 5);

  LCH_List *fields = LCH_CSVParseRecord("two,four");
  ck_assert_ptr_nonnull(fields);
  ck_assert_int_eq(LCH_ListLength(fields), 2);

  LCH_List *indices = GetIndexOfFields(header, fields);
  ck_assert_ptr_nonnull(indices);
  ck_assert_int_eq(LCH_ListLength(indices), 2);

  LCH_ListDestroy(fields);
  fields = ExtractFieldsAtIndices(header, indices);
  ck_assert_ptr_nonnull(fields);
  ck_assert_int_eq(LCH_ListLength(fields), 2);
  ck_assert_str_eq((char *)LCH_ListGet(fields, 0), "two");
  ck_assert_str_eq((char *)LCH_ListGet(fields, 1), "four");

  LCH_ListDestroy(header);
  LCH_ListDestroy(fields);
  LCH_ListDestroy(indices);
}
END_TEST

START_TEST(test_ParseConcat) {
  LCH_List *list = ParseConcatFields("one,two", "three,four", false);
  ck_assert_ptr_nonnull(list);
  ck_assert_int_eq(LCH_ListLength(list), 4);
  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "one");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "two");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "three");
  ck_assert_str_eq((char *)LCH_ListGet(list, 3), "four");
  LCH_ListDestroy(list);
  list = NULL;

  list = ParseConcatFields("one", "two,three,four", false);
  ck_assert_ptr_nonnull(list);
  ck_assert_int_eq(LCH_ListLength(list), 4);
  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "one");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "two");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "three");
  ck_assert_str_eq((char *)LCH_ListGet(list, 3), "four");
  LCH_ListDestroy(list);
  list = NULL;

  list = ParseConcatFields("one,two,three", "four", false);
  ck_assert_ptr_nonnull(list);
  ck_assert_int_eq(LCH_ListLength(list), 4);
  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "one");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "two");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "three");
  ck_assert_str_eq((char *)LCH_ListGet(list, 3), "four");
  LCH_ListDestroy(list);
  list = NULL;

  list = ParseConcatFields("one,two,three,four", NULL, false);
  ck_assert_ptr_nonnull(list);
  ck_assert_int_eq(LCH_ListLength(list), 4);
  ck_assert_str_eq((char *)LCH_ListGet(list, 0), "one");
  ck_assert_str_eq((char *)LCH_ListGet(list, 1), "two");
  ck_assert_str_eq((char *)LCH_ListGet(list, 2), "three");
  ck_assert_str_eq((char *)LCH_ListGet(list, 3), "four");
  LCH_ListDestroy(list);
  list = NULL;
}
END_TEST

START_TEST(test_LCH_StartsWith) {
  ck_assert(LCH_StringStartsWith("Hello World", "Hello"));
  ck_assert(!LCH_StringStartsWith("World", "Hello"));
  ck_assert(!LCH_StringStartsWith("Hello", "Hello World"));
  ck_assert(!LCH_StringStartsWith("", "Hello World"));
  ck_assert(LCH_StringStartsWith("Hello World", ""));
}
END_TEST

START_TEST(test_LCH_StringStrip) {
  char test1[] = "Hello World";
  ck_assert_str_eq(LCH_StringStrip(test1, " "), "Hello World");
  char test2[] = " \tHello\tWorld";
  ck_assert_str_eq(LCH_StringStrip(test2, " \t"), "Hello\tWorld");
  char test3[] = "Hello World\t";
  ck_assert_str_eq(LCH_StringStrip(test3, "\t "), "Hello World");
  char test4[] = " Hello World ";
  ck_assert_str_eq(LCH_StringStrip(test4, " "), "Hello World");
  char test5[] = "   Hello World     ";
  ck_assert_str_eq(LCH_StringStrip(test5, " "), "Hello World");
}
END_TEST

START_TEST(test_LCH_SplitString) {
  const char *expected[] = {"+,a,b,c", "-,e,d,f"};
  LCH_List *lst = LCH_SplitString("+,a,b,c\r\n-,e,d,f\r\n", "\r\n");
  ck_assert_str_eq((char *)LCH_ListGet(lst, 0), expected[0]);
  ck_assert_str_eq((char *)LCH_ListGet(lst, 1), expected[1]);
  LCH_ListDestroy(lst);
}
END_TEST

START_TEST(test_LCH_SplitStringSubstring) {
  {
    const char *expected[] = {"+,a,b,c\r\n-,e,d,f", "%%,g,h,i"};
    LCH_List *lst = LCH_SplitStringSubstring(
        "+,a,b,c\r\n-,e,d,f\r\n\r\n%%,g,h,i", "\r\n\r\n");
    ck_assert_str_eq((char *)LCH_ListGet(lst, 0), expected[0]);
    ck_assert_str_eq((char *)LCH_ListGet(lst, 1), expected[1]);
    LCH_ListDestroy(lst);
  }
  {
    const char *expected[] = {"+,a,b,c\r\n-,e,d,f", "%%,g,h,i"};
    LCH_List *lst = LCH_SplitStringSubstring(
        "\r\n\r\n+,a,b,c\r\n-,e,d,f\r\n\r\n%%,g,h,i", "\r\n\r\n");
    ck_assert_str_eq((char *)LCH_ListGet(lst, 0), expected[0]);
    ck_assert_str_eq((char *)LCH_ListGet(lst, 1), expected[1]);
    LCH_ListDestroy(lst);
  }
  {
    const char *expected[] = {"+,a,b,c\r\n-,e,d,f", "%%,g,h,i"};
    LCH_List *lst = LCH_SplitStringSubstring(
        "+,a,b,c\r\n-,e,d,f\r\n\r\n%%,g,h,i\r\n\r\n", "\r\n\r\n");
    ck_assert_str_eq((char *)LCH_ListGet(lst, 0), expected[0]);
    ck_assert_str_eq((char *)LCH_ListGet(lst, 1), expected[1]);
    LCH_ListDestroy(lst);
  }
}
END_TEST

START_TEST(test_LCH_PathJoin) {
  char path[PATH_MAX];
  ck_assert(
      LCH_PathJoin(path, sizeof(path), 3, ".leech", "snapshots", "beatles"));
  ck_assert_str_eq(path, ".leech/snapshots/beatles");
}
END_TEST

START_TEST(test_LCH_ReadWriteTextFile) {
  char path[] = "testfile";

  char expected[] = "Hello World!";
  ck_assert(LCH_FileWrite(path, expected));

  size_t size;
  char *actual = LCH_FileRead("testfile", &size);
  ck_assert_str_eq(expected, actual);
  ck_assert_int_eq(size, strlen(expected));
  free(actual);
  remove(path);
}
END_TEST

START_TEST(test_GetIndexOfFields) {
  LCH_List *const header = LCH_CSVParseRecord("zero,one,two,three,four,five");
  LCH_List *const fields = LCH_CSVParseRecord("two,four");
  LCH_List *const indices = GetIndexOfFields(header, fields);
  ck_assert_ptr_nonnull(indices);
  ck_assert_int_eq(LCH_ListLength(indices), 2);
  ck_assert_int_eq((size_t)LCH_ListGet(indices, 0), 2);
  ck_assert_int_eq((size_t)LCH_ListGet(indices, 1), 4);
  LCH_ListDestroy(header);
  LCH_ListDestroy(fields);
  LCH_ListDestroy(indices);
}
END_TEST

START_TEST(test_LCH_TableToDict) {
  const char *const csv =
      "firstname, lastname,  born\r\n"
      "Paul,      McCartney, 1942\r\n"
      "Ringo,     Starr,     1940\r\n"
      "John,      Lennon,    1940\r\n"
      "George,    Harrison,  1943\r\n";
  LCH_List *table = LCH_CSVParseTable(csv);
  LCH_Dict *dict = LCH_TableToDict(table, "lastname,firstname", "born", true);

  LCH_ListDestroy(table);

  ck_assert_ptr_nonnull(dict);
  ck_assert_str_eq((char *)LCH_DictGet(dict, "Paul,McCartney"), "1942");
  ck_assert_str_eq((char *)LCH_DictGet(dict, "Ringo,Starr"), "1940");
  ck_assert_str_eq((char *)LCH_DictGet(dict, "John,Lennon"), "1940");
  ck_assert_str_eq((char *)LCH_DictGet(dict, "George,Harrison"), "1943");

  LCH_DictDestroy(dict);
}
END_TEST

START_TEST(test_LCH_DictToTable) {
  const char *const csv =
      "firstname, lastname,  born\r\n"
      "Paul,      McCartney, 1942\r\n"
      "Ringo,     Starr,     1940\r\n"
      "John,      Lennon,    1940\r\n"
      "George,    Harrison,  1943\r\n";
  LCH_List *table = LCH_CSVParseTable(csv);
  LCH_Dict *dict = LCH_TableToDict(table, "firstname", "born,lastname", true);
  LCH_ListDestroy(table);

  table = LCH_DictToTable(dict, "firstname", "lastname,born", true);
  ck_assert_ptr_nonnull(table);
  LCH_DictDestroy(dict);

  size_t num_records = LCH_ListLength(table);
  ck_assert_int_eq(num_records, 5);

  for (size_t i = 0; i < num_records; i++) {
    LCH_List *record = (LCH_List *)LCH_ListGet(table, i);
    ck_assert_ptr_nonnull(record);
    ck_assert_int_eq(LCH_ListLength(record), 3);

    char *first = (char *)LCH_ListGet(record, 0);
    ck_assert_ptr_nonnull(first);

    char *second = (char *)LCH_ListGet(record, 1);
    ck_assert_ptr_nonnull(second);

    char *third = (char *)LCH_ListGet(record, 2);
    ck_assert_ptr_nonnull(third);

    if (i == 0) {
      ck_assert_str_eq(first, "firstname");
      ck_assert_str_eq(second, "born");
      ck_assert_str_eq(third, "lastname");
    } else if (strcmp(first, "Paul") == 0) {
      ck_assert_str_eq(second, "1942");
      ck_assert_str_eq(third, "McCartney");
    } else if (strcmp(first, "Ringo") == 0) {
      ck_assert_str_eq(second, "1940");
      ck_assert_str_eq(third, "Starr");
    } else if (strcmp(first, "John") == 0) {
      ck_assert_str_eq(second, "1940");
      ck_assert_str_eq(third, "Lennon");
    } else if (strcmp(first, "George") == 0) {
      ck_assert_str_eq(second, "1943");
      ck_assert_str_eq(third, "Harrison");
    } else {
      assert(false);
    }
  }

  LCH_ListDestroy(table);
}
END_TEST

Suite *UtilsSuite(void) {
  Suite *s = suite_create("utils.c");
  {
    TCase *tc = tcase_create("ExtractFieldsAtIndices");
    tcase_add_test(tc, test_ExtractFieldsAtIndices);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("ParseConcat");
    tcase_add_test(tc, test_ParseConcat);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_SplitString");
    tcase_add_test(tc, test_LCH_SplitString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_SplitStringSubstring");
    tcase_add_test(tc, test_LCH_SplitStringSubstring);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_StartsWith");
    tcase_add_test(tc, test_LCH_StartsWith);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_StringStrip");
    tcase_add_test(tc, test_LCH_StringStrip);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_PathJoin");
    tcase_add_test(tc, test_LCH_PathJoin);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_ReadWriteTextFile");
    tcase_add_test(tc, test_LCH_ReadWriteTextFile);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("GetIndexOfFields");
    tcase_add_test(tc, test_GetIndexOfFields);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_TableToDict");
    tcase_add_test(tc, test_LCH_TableToDict);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_DictToTable");
    tcase_add_test(tc, test_LCH_DictToTable);
    suite_add_tcase(s, tc);
  }
  return s;
}
