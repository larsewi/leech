#include <check.h>
#include <stdbool.h>

#include "../lib/json.c"

START_TEST(test_JsonParseNull) {
  const char *str = "nullnulltull";
  LCH_Json *json;
  str = JsonParseNull(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_str_eq(str, "nulltull");
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NULL);
  LCH_JsonDestroy(json);

  str = JsonParseNull(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_str_eq(str, "tull");
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NULL);
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_JsonParseTrue) {
  const char *str = "truetruetull";
  LCH_Json *json;
  str = JsonParseTrue(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_str_eq(str, "truetull");
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_TRUE);
  LCH_JsonDestroy(json);

  str = JsonParseTrue(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_str_eq(str, "tull");
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_TRUE);
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_JsonParseFalse) {
  const char *str = "falsefalsetull";
  LCH_Json *json;
  str = JsonParseFalse(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_str_eq(str, "falsetull");
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_FALSE);
  LCH_JsonDestroy(json);

  str = JsonParseFalse(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_str_eq(str, "tull");
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_FALSE);
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_JsonParseString) {
  const char *str =
      "\"Hello World\""
      "\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t \""
      "\"\\u0041\\u0100\\u0101\""
      "bogus";
  LCH_Json *json;

  str = JsonParseString(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(json), "Hello World");
  LCH_JsonDestroy(json);

  str = JsonParseString(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(json), " \" \\ / \b \f \n \r \t ");
  LCH_JsonDestroy(json);

  str = JsonParseString(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(json), "AĀā");
  LCH_JsonDestroy(json);

  ck_assert_str_eq(str, "bogus");
}
END_TEST

START_TEST(test_JsonParseObject) {
  const char *str =
      "{}"
      "{ }"
      "{ \"name\" : \"leech\" }"
      "{ \"name\" : \"leech\", \"version\":\"1.2.3\"}"
      "bogus";
  LCH_Json *json;

  str = JsonParseObject(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_OBJECT);
  ck_assert_int_eq(LCH_JsonObjectLength(json), 0);
  LCH_JsonDestroy(json);

  str = JsonParseObject(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_OBJECT);
  ck_assert_int_eq(LCH_JsonObjectLength(json), 0);
  LCH_JsonDestroy(json);

  str = JsonParseObject(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_OBJECT);
  ck_assert_int_eq(LCH_JsonObjectLength(json), 1);
  const LCH_Json *child = LCH_JsonObjectGet(json, "name");
  ck_assert_ptr_nonnull(child);
  ck_assert_int_eq(LCH_JsonGetType(child), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(child), "leech");
  LCH_JsonDestroy(json);

  str = JsonParseObject(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_OBJECT);
  ck_assert_int_eq(LCH_JsonObjectLength(json), 2);
  child = LCH_JsonObjectGet(json, "name");
  ck_assert_ptr_nonnull(child);
  ck_assert_int_eq(LCH_JsonGetType(child), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(child), "leech");
  child = LCH_JsonObjectGet(json, "version");
  ck_assert_ptr_nonnull(child);
  ck_assert_int_eq(LCH_JsonGetType(child), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(child), "1.2.3");
  LCH_JsonDestroy(json);

  ck_assert_str_eq(str, "bogus");
}
END_TEST

START_TEST(test_JsonParseList) {
  const char *str =
      "[]"
      "[ ]"
      "[\"leech\"]"
      "[\"leech\" , \"1.2.3\"]"
      "bogus";
  LCH_Json *json;
  const LCH_Json *child;

  str = JsonParseArray(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_ARRAY);
  ck_assert_int_eq(LCH_JsonListLength(json), 0);
  LCH_JsonDestroy(json);

  str = JsonParseArray(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_ARRAY);
  ck_assert_int_eq(LCH_JsonListLength(json), 0);
  LCH_JsonDestroy(json);

  str = JsonParseArray(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_ARRAY);
  ck_assert_int_eq(LCH_JsonListLength(json), 1);
  child = LCH_JsonArrayGet(json, 0);
  ck_assert_int_eq(LCH_JsonGetType(child), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(child), "leech");
  LCH_JsonDestroy(json);

  str = JsonParseArray(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_ARRAY);
  ck_assert_int_eq(LCH_JsonListLength(json), 2);
  child = LCH_JsonArrayGet(json, 0);
  ck_assert_int_eq(LCH_JsonGetType(child), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(child), "leech");
  child = LCH_JsonArrayGet(json, 1);
  ck_assert_int_eq(LCH_JsonGetType(child), LCH_JSON_TYPE_STRING);
  ck_assert_str_eq(LCH_JsonStringGet(child), "1.2.3");
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_JsonParseNumber) {
  const char *str =
      "123"
      " 123.45"
      " -123.45"
      " 123e4"
      " 123E-4"
      " 123E+4"
      " 123.4e-5";
  LCH_Json *json;

  str = JsonParseNumber(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  float number = LCH_JsonNumberGet(json);
  ck_assert_float_eq(number, 123.0f);
  LCH_JsonDestroy(json);

  str = JsonParseNumber(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  number = LCH_JsonNumberGet(json);
  ck_assert_float_eq(number, 123.45f);
  LCH_JsonDestroy(json);

  str = JsonParseNumber(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  number = LCH_JsonNumberGet(json);
  ck_assert_float_eq(number, -123.45f);
  LCH_JsonDestroy(json);

  str = JsonParseNumber(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  number = LCH_JsonNumberGet(json);
  ck_assert_float_eq(number, 123e4f);
  LCH_JsonDestroy(json);

  str = JsonParseNumber(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  number = LCH_JsonNumberGet(json);
  ck_assert_float_eq(number, 123e-4f);
  LCH_JsonDestroy(json);

  str = JsonParseNumber(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  number = LCH_JsonNumberGet(json);
  ck_assert_float_eq(number, 123e4f);
  LCH_JsonDestroy(json);

  str = JsonParseNumber(str, &json);
  ck_assert_ptr_nonnull(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  number = LCH_JsonNumberGet(json);
  ck_assert_float_eq(number, 123.4e-5f);
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonParse) {
  const char *const str =
      " { "
      "\"name\": \"lars\","
      "\"height\": 185.3,"
      "\"skills\": [ \"C\", \"C++\" ] ,"
      "\"awesome\": true"
      " } ";
  LCH_Json *json = LCH_JsonParse(str);
  float height = LCH_JsonNumberGet(LCH_JsonObjectGet(json, "height"));
  ck_assert_float_eq(height, 185.3f);
  const char *skill =
      LCH_JsonStringGet(LCH_JsonArrayGet(LCH_JsonObjectGet(json, "skills"), 1));
  ck_assert_str_eq(skill, "C++");
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_JsonComposeNull) {
  LCH_Json *json = LCH_JsonCreateNull();
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NULL);
  char *const str = LCH_JsonCompose(json);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "null");
  LCH_JsonDestroy(json);
  free(str);
}
END_TEST

START_TEST(test_JsonComposeTrue) {
  LCH_Json *json = LCH_JsonCreateTrue();
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_TRUE);
  char *const str = LCH_JsonCompose(json);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "true");
  LCH_JsonDestroy(json);
  free(str);
}
END_TEST

START_TEST(test_JsonComposeFalse) {
  LCH_Json *json = LCH_JsonCreateFalse();
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_FALSE);
  char *const str = LCH_JsonCompose(json);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "false");
  LCH_JsonDestroy(json);
  free(str);
}
END_TEST

START_TEST(test_JsonComposeString) {
  char *str = strdup("Leech");
  ck_assert_ptr_nonnull(str);
  LCH_Json *json = LCH_JsonCreateString(str);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_STRING);
  str = LCH_JsonCompose(json);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "\"Leech\"");
  LCH_JsonDestroy(json);
  free(str);
}
END_TEST

START_TEST(test_JsonComposeNumber) {
  LCH_Json *json = LCH_JsonCreateNumber(1.012300f);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_NUMBER);
  char *const str = LCH_JsonCompose(json);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "1.0123");
  LCH_JsonDestroy(json);
  free(str);
}
END_TEST

START_TEST(test_JsonComposeList) {
  const char values[][8] = {"one", "two", "three"};
  LCH_List *const list = LCH_ListCreate();
  ck_assert_ptr_nonnull(list);
  for (size_t i = 0; i < 3; i++) {
    char *value = strdup(values[i]);
    ck_assert_ptr_nonnull(value);
    LCH_Json *json = LCH_JsonCreateString(value);
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_ListAppend(list, json, (void (*)(void *))LCH_JsonDestroy));
  }

  LCH_Json *json = LCH_JsonArrayCreateFromList(list);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_ARRAY);
  char *const str = LCH_JsonCompose(json);
  ck_assert_ptr_nonnull(str);
  ck_assert_str_eq(str, "[\"one\",\"two\",\"three\"]");
  LCH_JsonDestroy(json);
  free(str);
}
END_TEST

START_TEST(test_JsonComposeObject) {
  LCH_Dict *const dict = LCH_DictCreate();
  ck_assert_ptr_nonnull(dict);
  char *const lars = strdup("lars");
  ck_assert_ptr_nonnull(lars);
  LCH_Json *const name = LCH_JsonCreateString(lars);
  ck_assert(LCH_DictSet(dict, "name", name, (void (*)(void *))LCH_JsonDestroy));
  LCH_Json *const age = LCH_JsonCreateNumber(29.0f);
  ck_assert_ptr_nonnull(age);
  ck_assert(LCH_DictSet(dict, "age", age, (void (*)(void *))LCH_JsonDestroy));

  LCH_Json *const json = LCH_JsonObjectCreateFromDict(dict);
  ck_assert_ptr_nonnull(json);
  ck_assert_int_eq(LCH_JsonGetType(json), LCH_JSON_TYPE_OBJECT);
  char *const str = LCH_JsonCompose(json);
  ck_assert(strcmp(str, "{\"name\":\"lars\",\"age\":29}") == 0 ||
            strcmp(str, "{\"age\":29},\"name\":\"lars\"") == 0);
  LCH_JsonDestroy(json);
  free(str);
}
END_TEST

Suite *JSONSuite(void) {
  Suite *s = suite_create("json.c");
  {
    TCase *tc = tcase_create("JsonParseNull");
    tcase_add_test(tc, test_JsonParseNull);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonParseTrue");
    tcase_add_test(tc, test_JsonParseTrue);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonParseFalse");
    tcase_add_test(tc, test_JsonParseFalse);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonParseString");
    tcase_add_test(tc, test_JsonParseString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonParseObject");
    tcase_add_test(tc, test_JsonParseObject);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonParseList");
    tcase_add_test(tc, test_JsonParseList);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonParseNumber");
    tcase_add_test(tc, test_JsonParseNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParse");
    tcase_add_test(tc, test_LCH_JsonParse);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonComposeNull");
    tcase_add_test(tc, test_JsonComposeNull);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonComposeTrue");
    tcase_add_test(tc, test_JsonComposeTrue);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonComposeFalse");
    tcase_add_test(tc, test_JsonComposeFalse);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonComposeString");
    tcase_add_test(tc, test_JsonComposeString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonComposeNumber");
    tcase_add_test(tc, test_JsonComposeNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonComposeList");
    tcase_add_test(tc, test_JsonComposeList);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("JsonComposeObject");
    tcase_add_test(tc, test_JsonComposeObject);
    suite_add_tcase(s, tc);
  }
  return s;
}
