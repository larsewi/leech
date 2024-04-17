#include <check.h>

#include "../lib/json.h"
#include "../lib/string_lib.h"

START_TEST(test_LCH_JsonXXXXXCreate) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const jsons[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonNumberCreate(1337.0),
      LCH_JsonStringCreate(buffer), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  const size_t num_jsons = sizeof(jsons) / sizeof(LCH_Json *);
  for (size_t i = 0; i < num_jsons; i++) {
    ck_assert_ptr_nonnull(jsons[i]);
    LCH_JsonDestroy(jsons[i]);
  }
}
END_TEST

START_TEST(test_LCH_JsonGetType) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const jsons[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  const LCH_JsonType types[] = {
      LCH_JSON_TYPE_NULL,   LCH_JSON_TYPE_TRUE,   LCH_JSON_TYPE_FALSE,
      LCH_JSON_TYPE_STRING, LCH_JSON_TYPE_NUMBER, LCH_JSON_TYPE_ARRAY,
      LCH_JSON_TYPE_OBJECT,
  };

  const size_t num_jsons = sizeof(jsons) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_jsons, sizeof(types) / sizeof(LCH_JsonType));

  for (size_t i = 0; i < num_jsons; i++) {
    ck_assert_ptr_nonnull(jsons[i]);
    for (size_t j = 0; j < num_jsons; j++) {
      if (i == j) {
        ck_assert_int_eq(LCH_JsonGetType(jsons[i]), types[j]);
      } else {
        ck_assert_int_ne(LCH_JsonGetType(jsons[i]), types[j]);
      }
    }
    LCH_JsonDestroy(jsons[i]);
  }
}
END_TEST

START_TEST(test_LCH_JsonGetTypeAsString) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const jsons[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  const char *const types[] = {
      "null", "true", "false", "string", "number", "array", "object",
  };

  const size_t num_jsons = sizeof(jsons) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_jsons, sizeof(types) / sizeof(char *));

  for (size_t i = 0; i < num_jsons; i++) {
    ck_assert_ptr_nonnull(jsons[i]);
    for (size_t j = 0; j < num_jsons; j++) {
      if (i == j) {
        ck_assert_str_eq(LCH_JsonGetTypeAsString(jsons[i]), types[j]);
      } else {
        ck_assert_str_ne(LCH_JsonGetTypeAsString(jsons[i]), types[j]);
      }
    }
    LCH_JsonDestroy(jsons[i]);
  }
}
END_TEST

START_TEST(test_LCH_JsonIsXXXXX) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const jsons[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonNumberCreate(1337.0),
      LCH_JsonStringCreate(buffer), LCH_JsonObjectCreate(),
      LCH_JsonArrayCreate(),
  };

  bool (*funcs[])(const LCH_Json *) = {
      LCH_JsonIsNull,   LCH_JsonIsTrue,   LCH_JsonIsFalse, LCH_JsonIsNumber,
      LCH_JsonIsString, LCH_JsonIsObject, LCH_JsonIsArray,
  };

  const size_t num_jsons = sizeof(jsons) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_jsons, sizeof(funcs) / sizeof(bool (*)(LCH_Json *)));

  for (size_t i = 0; i < num_jsons; i++) {
    ck_assert_ptr_nonnull(jsons[i]);
    for (size_t j = 0; j < num_jsons; j++) {
      if (i == j) {
        ck_assert(funcs[j](jsons[i]));
      } else {
        ck_assert(!(funcs[j](jsons[i])));
      }
    }
    LCH_JsonDestroy(jsons[i]);
  }
}
END_TEST

START_TEST(test_LCH_JsonObjectChildIsXXXXX) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonNumberCreate(1337.0),
      LCH_JsonStringCreate(buffer), LCH_JsonObjectCreate(),
      LCH_JsonArrayCreate(),
  };

  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);

  const char *const keys[] = {"null",   "true",   "false", "number",
                              "string", "object", "array"};

  bool (*funcs[])(const LCH_Json *, const LCH_Buffer *) = {
      LCH_JsonObjectChildIsNull,   LCH_JsonObjectChildIsTrue,
      LCH_JsonObjectChildIsFalse,  LCH_JsonObjectChildIsNumber,
      LCH_JsonObjectChildIsString, LCH_JsonObjectChildIsObject,
      LCH_JsonObjectChildIsArray,
  };

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_children, sizeof(keys) / sizeof(char *));
  ck_assert_int_eq(num_children,
                   sizeof(funcs) / sizeof(bool (*)(LCH_Json *, LCH_Buffer *)));

  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    const LCH_Buffer *const key = LCH_BufferStaticFromString(keys[i]);
    ck_assert(LCH_JsonObjectSet(parent, key, children[i]));

    for (size_t j = 0; j < num_children; j++) {
      if (i == j) {
        ck_assert(funcs[j](parent, key));
      } else {
        ck_assert(!funcs[j](parent, key));
      }
    }
  }
  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonArrayChildIsXXXXX) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonNumberCreate(1337.0),
      LCH_JsonStringCreate(buffer), LCH_JsonObjectCreate(),
      LCH_JsonArrayCreate(),
  };

  LCH_Json *const parent = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(parent);

  bool (*funcs[])(const LCH_Json *, size_t) = {
      LCH_JsonArrayChildIsNull,   LCH_JsonArrayChildIsTrue,
      LCH_JsonArrayChildIsFalse,  LCH_JsonArrayChildIsNumber,
      LCH_JsonArrayChildIsString, LCH_JsonArrayChildIsObject,
      LCH_JsonArrayChildIsArray,
  };

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_children,
                   sizeof(funcs) / sizeof(bool (*)(LCH_Json *, size_t)));

  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    ck_assert(LCH_JsonArrayAppend(parent, children[i]));
    for (size_t j = 0; j < num_children; j++) {
      if (i == j) {
        ck_assert(funcs[j](parent, i));
      } else {
        ck_assert(!funcs[j](parent, i));
      }
    }
  }
  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonObjectSet) {
  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);
  LCH_Json *const child = LCH_JsonNullCreate();
  ck_assert_ptr_nonnull(child);
  ck_assert(
      LCH_JsonObjectSet(parent, LCH_BufferStaticFromString("leech"), child));
  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonObjectSetString) {
  LCH_Json *const json = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(json);
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);
  ck_assert(LCH_JsonObjectSetString(json, LCH_BufferStaticFromString("leech"),
                                    buffer));
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonObjectSetStringDuplicate) {
  LCH_Json *const json = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(json);
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);
  ck_assert(LCH_JsonObjectSetStringDuplicate(
      json, LCH_BufferStaticFromString("leech"), buffer));
  LCH_BufferDestroy(buffer);
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonObjectSetNumber) {
  LCH_Json *const json = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(json);
  ck_assert(LCH_JsonObjectSetNumber(json, LCH_BufferStaticFromString("leech"),
                                    1337.0));
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonArrayAppend) {
  LCH_Json *const parent = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(parent);
  LCH_Json *const child = LCH_JsonNullCreate();
  ck_assert_ptr_nonnull(child);
  ck_assert(LCH_JsonArrayAppend(parent, child));
  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonArrayAppendString) {
  LCH_Json *const json = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(json);
  LCH_Buffer *const str = LCH_BufferCreate();
  ck_assert_ptr_nonnull(str);
  ck_assert(LCH_JsonArrayAppendString(json, str));
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonNumberGet) {
  LCH_Json *const json = LCH_JsonNumberCreate(1337.0);
  ck_assert_ptr_nonnull(json);
  const double number = LCH_JsonNumberGet(json);
  ck_assert_double_eq(number, 1337.0);
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonObjectGet) {
  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);
  LCH_Json *const child = LCH_JsonNullCreate();
  ck_assert_ptr_nonnull(child);
  ck_assert(
      LCH_JsonObjectSet(parent, LCH_BufferStaticFromString("foo"), child));
  ck_assert_ptr_eq(LCH_JsonObjectGet(parent, LCH_BufferStaticFromString("foo")),
                   child);
  ck_assert_ptr_null(
      LCH_JsonObjectGet(parent, LCH_BufferStaticFromString("bar")));
  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonArrayGet) {
  LCH_Json *const parent = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(parent);
  LCH_Json *const child = LCH_JsonNullCreate();
  ck_assert_ptr_nonnull(child);
  ck_assert(LCH_JsonArrayAppend(parent, child));
  ck_assert_ptr_eq(LCH_JsonArrayGet(parent, 0), child);
  ck_assert_ptr_null(LCH_JsonArrayGet(parent, 1));
  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonStringGet) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);
  LCH_Json *const json = LCH_JsonStringCreate(buffer);
  ck_assert_ptr_nonnull(json);
  ck_assert_ptr_eq(LCH_JsonStringGet(json), buffer);
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonObjectGetString) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);

  const char *const keys[] = {"null",   "true",  "false", "string",
                              "number", "array", "object"};

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_children, sizeof(keys) / sizeof(char *));

  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    const LCH_Buffer *const key = LCH_BufferStaticFromString(keys[i]);
    ck_assert(LCH_JsonObjectSet(parent, key, children[i]));

    if (LCH_StringEqual(keys[i], "string")) {
      ck_assert_ptr_eq(LCH_JsonObjectGetString(parent, key), buffer);
    } else {
      ck_assert_ptr_null(LCH_JsonObjectGetString(parent, key));
    }
  }
  ck_assert_ptr_null(
      LCH_JsonObjectGetString(parent, LCH_BufferStaticFromString("bogus")));

  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonArrayGetString) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  LCH_Json *const parent = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(parent);

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    ck_assert(LCH_JsonArrayAppend(parent, children[i]));

    if (i == 3) {
      ck_assert_ptr_eq(LCH_JsonArrayGetString(parent, i), buffer);
    } else {
      ck_assert_ptr_null(LCH_JsonArrayGetString(parent, i));
    }
  }
  ck_assert_ptr_null(LCH_JsonArrayGetString(parent, num_children));

  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonObjectGetObject) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);

  const char *const keys[] = {"null",   "true",  "false", "string",
                              "number", "array", "object"};

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_children, sizeof(keys) / sizeof(char *));

  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    const LCH_Buffer *const key = LCH_BufferStaticFromString(keys[i]);
    ck_assert(LCH_JsonObjectSet(parent, key, children[i]));

    if (i == 6) {
      ck_assert_ptr_eq(LCH_JsonObjectGetObject(parent, key), children[i]);
    } else {
      ck_assert_ptr_null(LCH_JsonObjectGetObject(parent, key));
    }
  }
  ck_assert_ptr_null(
      LCH_JsonObjectGetObject(parent, LCH_BufferStaticFromString("bogus")));

  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonObjectGetNumber) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);

  const char *const keys[] = {"null",   "true",  "false", "string",
                              "number", "array", "object"};

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_children, sizeof(keys) / sizeof(char *));

  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    const LCH_Buffer *const key = LCH_BufferStaticFromString(keys[i]);
    ck_assert(LCH_JsonObjectSet(parent, key, children[i]));

    double number = 42.0;
    if (i == 4) {
      ck_assert(LCH_JsonObjectGetNumber(parent, key, &number));
      ck_assert_double_eq(number, 1337.0);
    } else {
      ck_assert(!LCH_JsonObjectGetNumber(parent, key, &number));
      ck_assert_double_eq(number, 42.0);
    }
  }
  ck_assert_ptr_null(
      LCH_JsonObjectGetObject(parent, LCH_BufferStaticFromString("bogus")));

  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonArrayGetObject) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  LCH_Json *const parent = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(parent);

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    ck_assert(LCH_JsonArrayAppend(parent, children[i]));

    if (i == 6) {
      ck_assert_ptr_eq(LCH_JsonArrayGetObject(parent, i), children[i]);
    } else {
      ck_assert_ptr_null(LCH_JsonArrayGetObject(parent, i));
    }
  }
  ck_assert_ptr_null(LCH_JsonArrayGetObject(parent, num_children));

  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonObjectGetArray) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  LCH_Json *const children[] = {
      LCH_JsonNullCreate(),         LCH_JsonTrueCreate(),
      LCH_JsonFalseCreate(),        LCH_JsonStringCreate(buffer),
      LCH_JsonNumberCreate(1337.0), LCH_JsonArrayCreate(),
      LCH_JsonObjectCreate(),
  };

  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);

  const char *const keys[] = {"null",   "true",  "false", "string",
                              "number", "array", "object"};

  const size_t num_children = sizeof(children) / sizeof(LCH_Json *);
  ck_assert_int_eq(num_children, sizeof(keys) / sizeof(char *));

  for (size_t i = 0; i < num_children; i++) {
    ck_assert_ptr_nonnull(children[i]);
    const LCH_Buffer *const key = LCH_BufferStaticFromString(keys[i]);
    ck_assert(LCH_JsonObjectSet(parent, key, children[i]));

    if (i == 5) {
      ck_assert_ptr_eq(LCH_JsonObjectGetArray(parent, key), children[i]);
    } else {
      ck_assert_ptr_null(LCH_JsonObjectGetArray(parent, key));
    }
  }
  ck_assert_ptr_null(
      LCH_JsonObjectGetArray(parent, LCH_BufferStaticFromString("bogus")));

  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonObjectGetKeys) {
  LCH_Json *const parent = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(parent);

  const char *const keys[] = {"foo", "bar", "baz"};
  const size_t num_keys = sizeof(keys) / sizeof(char *);

  for (size_t i = 0; i < num_keys; i++) {
    LCH_Json *const child = LCH_JsonNullCreate();
    ck_assert_ptr_nonnull(child);
    ck_assert(
        LCH_JsonObjectSet(parent, LCH_BufferStaticFromString(keys[i]), child));
  }

  LCH_List *const actual = LCH_JsonObjectGetKeys(parent);
  ck_assert_ptr_nonnull(actual);

  ck_assert_int_eq(num_keys, LCH_ListLength(actual));
  for (size_t i = 0; i < num_keys; i++) {
    ck_assert_int_lt(LCH_ListIndex(actual, LCH_BufferStaticFromString(keys[i]),
                                   (LCH_CompareFn)LCH_BufferCompare),
                     num_keys);
  }

  LCH_ListDestroy(actual);
  LCH_JsonDestroy(parent);
}
END_TEST

START_TEST(test_LCH_JsonObjectHasKey) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonObjectLength) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonArrayLength) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonObjectRemove) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonArrayRemove) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonObjectRemoveObject) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonObjectRemoveArray) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonArrayRemoveObject) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonArrayRemoveArray) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonParseNull) {
  const char *const valid[] = {
      "null", " null", "null ", " null ", "\r\n\t null \t\n\r", NULL};
  for (size_t i = 0; valid[i] != NULL; i++) {
    LCH_Json *const json = LCH_JsonParse(valid[i], strlen(valid[i]));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsNull(json));
    LCH_JsonDestroy(json);
  }
}

START_TEST(test_LCH_JsonParseTrue) {
  const char *const strs[] = {
      "true", " true", "true ", " true ", "\r\n\t true \t\n\r", NULL};
  for (size_t i = 0; strs[i] != NULL; i++) {
    LCH_Json *const json = LCH_JsonParse(strs[i], strlen(strs[i]));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsTrue(json));
    LCH_JsonDestroy(json);
  }
}
END_TEST

START_TEST(test_LCH_JsonParseFalse) {
  const char *const strs[] = {
      "false", " false", "false ", " false ", "\r\n\t false \t\n\r", NULL};
  for (size_t i = 0; strs[i] != NULL; i++) {
    LCH_Json *const json = LCH_JsonParse(strs[i], strlen(strs[i]));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsFalse(json));
    LCH_JsonDestroy(json);
  }
}
END_TEST

START_TEST(test_LCH_JsonParseNumber) {
  const char *const strs[] = {"0",         "123.456", "-5.789012", "-987e-3",
                              "0.456E+10", "0.1e67",  NULL};
  const double nums[] = {0.0, 123.456, -5.789012, -987e-3, 0.456e+10, 0.1e67};
  for (size_t i = 0; strs[i] != NULL; i++) {
    LCH_Json *const json = LCH_JsonParse(strs[i], strlen(strs[i]));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsNumber(json));
    ck_assert_double_eq(nums[i], LCH_JsonNumberGet(json));
    LCH_JsonDestroy(json);
  }
}
END_TEST

START_TEST(test_LCH_JsonParseString) {
  const char *const strs[] = {"\"\"",     "\"leech\"", " \"foo\"",
                              "\"bar\" ", " \"baz\" ", NULL};
  const char *const expected[] = {"", "leech", "foo", "bar", "baz"};
  for (size_t i = 0; strs[i] != NULL; i++) {
    LCH_Json *const json = LCH_JsonParse(strs[i], strlen(strs[i]));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsString(json));
    const LCH_Buffer *const actual = LCH_JsonStringGet(json);
    ck_assert_ptr_nonnull(actual);
    ck_assert_str_eq(expected[i], LCH_BufferData(actual));
    LCH_JsonDestroy(json);
  }
}
END_TEST

START_TEST(test_LCH_JsonParseArray) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonParseObject) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonParse) {
  {
    const char *const str =
        "{"
        "  \"one\": \"two,\\\"three\\\"\","
        "  \"five\": \"six\""
        "}";
    LCH_Json *const json = LCH_JsonParse(str, strlen(str));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsObject(json));
    LCH_JsonDestroy(json);
  }
  {
    const char *const str =
        "{\"default,mount_units,mount_unit_show_items[run-snapd-ns][UID][0]\":"
        "\"0,UID,\\\"source=function,function=buildlinearray\\\"\",\"default,"
        "mount_units,mount_unit_show_items[dev-hugepages][LimitSIGPENDINGSoft]["
        "0]\":\"0,LimitSIGPENDINGSoft,\\\"source=function,function="
        "buildlinearray\\\"\"}";
    LCH_Json *const json = LCH_JsonParse(str, strlen(str));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsObject(json));
    LCH_JsonDestroy(json);
  }
  {
    const char *const str =
        "{\"default,mount_units,mount_unit_show_items[run-snapd-ns][UID][1]\":"
        "\"0,[not set],\\\"source=function,function=buildlinearray\\\"\"}";
    LCH_Json *const json = LCH_JsonParse(str, strlen(str));
    ck_assert_ptr_nonnull(json);
    ck_assert(LCH_JsonIsObject(json));
    LCH_JsonDestroy(json);
  }
}
END_TEST

START_TEST(test_LCH_JsonParseFile) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonComposeNull) {
  LCH_Json *const json = LCH_JsonNullCreate();
  ck_assert_ptr_nonnull(json);
  {
    LCH_Buffer *const buffer = LCH_JsonCompose(json, false);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_str_eq(LCH_BufferData(buffer), "null");
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_JsonCompose(json, true);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_str_eq(LCH_BufferData(buffer), "null\n");
    LCH_BufferDestroy(buffer);
  }
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonComposeTrue) {
  LCH_Json *const json = LCH_JsonTrueCreate();
  ck_assert_ptr_nonnull(json);
  {
    LCH_Buffer *const buffer = LCH_JsonCompose(json, false);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_str_eq(LCH_BufferData(buffer), "true");
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_JsonCompose(json, true);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_str_eq(LCH_BufferData(buffer), "true\n");
    LCH_BufferDestroy(buffer);
  }
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonComposeFalse) {
  LCH_Json *const json = LCH_JsonFalseCreate();
  ck_assert_ptr_nonnull(json);
  {
    LCH_Buffer *const buffer = LCH_JsonCompose(json, false);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_str_eq(LCH_BufferData(buffer), "false");
    LCH_BufferDestroy(buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_JsonCompose(json, true);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_str_eq(LCH_BufferData(buffer), "false\n");
    LCH_BufferDestroy(buffer);
  }
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonComposeNumber) {
  LCH_Json *const json = LCH_JsonNumberCreate(123.0);
  ck_assert_ptr_nonnull(json);
  {
    LCH_Buffer *const actual = LCH_JsonCompose(json, false);
    ck_assert_ptr_nonnull(actual);

    ck_assert_str_eq(LCH_BufferData(actual), "123.000000");
    LCH_BufferDestroy(actual);
  }
  {
    LCH_Buffer *const actual = LCH_JsonCompose(json, true);
    ck_assert_ptr_nonnull(actual);

    ck_assert_str_eq(LCH_BufferData(actual), "123.000000\n");
    LCH_BufferDestroy(actual);
  }
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonComposeString) {
  {
    LCH_Buffer *const str = LCH_BufferFromString("foo");
    ck_assert_ptr_nonnull(str);

    LCH_Json *const json = LCH_JsonStringCreate(str);
    ck_assert_ptr_nonnull(json);

    {
      LCH_Buffer *const actual = LCH_JsonCompose(json, false);
      ck_assert_ptr_nonnull(actual);

      ck_assert_str_eq(LCH_BufferData(actual), "\"foo\"");
      LCH_BufferDestroy(actual);
    }
    {
      LCH_Buffer *const actual = LCH_JsonCompose(json, true);
      ck_assert_ptr_nonnull(actual);

      ck_assert_str_eq(LCH_BufferData(actual), "\"foo\"\n");
      LCH_BufferDestroy(actual);
    }

    LCH_JsonDestroy(json);
  }
  {
    LCH_Buffer *const str = LCH_BufferFromString("\"bar\"");
    ck_assert_ptr_nonnull(str);

    LCH_Json *const json = LCH_JsonStringCreate(str);
    ck_assert_ptr_nonnull(json);

    {
      LCH_Buffer *const actual = LCH_JsonCompose(json, false);
      ck_assert_ptr_nonnull(actual);

      ck_assert_str_eq(LCH_BufferData(actual), "\"\\\"bar\\\"\"");
      LCH_BufferDestroy(actual);
    }
    {
      LCH_Buffer *const actual = LCH_JsonCompose(json, true);
      ck_assert_ptr_nonnull(actual);

      ck_assert_str_eq(LCH_BufferData(actual), "\"\\\"bar\\\"\"\n");
      LCH_BufferDestroy(actual);
    }

    LCH_JsonDestroy(json);
  }
}
END_TEST

START_TEST(test_LCH_JsonComposeArray) {
  LCH_Json *const json = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(json);
  {
    LCH_Buffer *const actual = LCH_JsonCompose(json, false);
    ck_assert_ptr_nonnull(actual);

    ck_assert_str_eq(LCH_BufferData(actual), "[]");
    LCH_BufferDestroy(actual);
  }
  {
    LCH_Buffer *const actual = LCH_JsonCompose(json, true);
    ck_assert_ptr_nonnull(actual);

    ck_assert_str_eq(LCH_BufferData(actual), "[\n]\n");
    LCH_BufferDestroy(actual);
  }
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonComposeObject) {
  LCH_Json *const json = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(json);
  {
    LCH_Buffer *const actual = LCH_JsonCompose(json, false);
    ck_assert_ptr_nonnull(json);

    ck_assert_str_eq(LCH_BufferData(actual), "{}");
    LCH_BufferDestroy(actual);
  }
  {
    LCH_Buffer *const actual = LCH_JsonCompose(json, true);
    ck_assert_ptr_nonnull(json);

    ck_assert_str_eq(LCH_BufferData(actual), "{\n}\n");
    LCH_BufferDestroy(actual);
  }
  LCH_JsonDestroy(json);
}
END_TEST

START_TEST(test_LCH_JsonCompose) {
  LCH_Json *const config = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(config);

  LCH_Buffer *const version = LCH_BufferFromString(PACKAGE_VERSION);
  ck_assert_ptr_nonnull(version);
  ck_assert(LCH_JsonObjectSetString(
      config, LCH_BufferStaticFromString("version"), version));

  const double max_chain_length = 64.0;
  ck_assert(LCH_JsonObjectSetNumber(
      config, LCH_BufferStaticFromString("max_chain_length"), 64.0));

  LCH_Json *const pretty_json = LCH_JsonTrueCreate();
  ck_assert_ptr_nonnull(pretty_json);
  ck_assert(LCH_JsonObjectSet(config, LCH_BufferStaticFromString("pretty_json"),
                              pretty_json));

  LCH_Json *const compression = LCH_JsonFalseCreate();
  ck_assert_ptr_nonnull(compression);
  ck_assert(LCH_JsonObjectSet(config, LCH_BufferStaticFromString("compression"),
                              compression));

  LCH_Json *const tables = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(tables);
  ck_assert(
      LCH_JsonObjectSet(config, LCH_BufferStaticFromString("tables"), tables));

  LCH_Json *const beatles = LCH_JsonObjectCreate();
  ck_assert_ptr_nonnull(beatles);
  ck_assert(
      LCH_JsonObjectSet(tables, LCH_BufferStaticFromString("BTL"), beatles));

  LCH_Json *const primary_fields = LCH_JsonArrayCreate();
  ck_assert_ptr_nonnull(primary_fields);
  ck_assert(LCH_JsonObjectSet(
      beatles, LCH_BufferStaticFromString("primary_fields"), primary_fields));

  LCH_Buffer *const first_name_buffer = LCH_BufferFromString("first_name");
  ck_assert_ptr_nonnull(first_name_buffer);
  LCH_Json *const first_name = LCH_JsonStringCreate(first_name_buffer);
  ck_assert_ptr_nonnull(first_name);
  ck_assert(LCH_JsonArrayAppend(primary_fields, first_name));

  LCH_Buffer *const last_name_buffer = LCH_BufferFromString("last_name");
  ck_assert_ptr_nonnull(last_name_buffer);
  LCH_Json *const last_name = LCH_JsonStringCreate(last_name_buffer);
  ck_assert_ptr_nonnull(last_name);
  ck_assert(LCH_JsonArrayAppend(primary_fields, last_name));

  LCH_Buffer *const born_buffer = LCH_BufferFromString("born");
  ck_assert_ptr_nonnull(born_buffer);
  LCH_Json *const born = LCH_JsonStringCreate(born_buffer);
  ck_assert_ptr_nonnull(born);
  ck_assert(LCH_JsonArrayAppend(primary_fields, born));

  LCH_Json *const subsidiary_fields = LCH_JsonNullCreate();
  ck_assert_ptr_nonnull(subsidiary_fields);
  ck_assert(LCH_JsonObjectSet(beatles,
                              LCH_BufferStaticFromString("subsidiary_fields"),
                              subsidiary_fields));

  LCH_Buffer *const actual = LCH_JsonCompose(config, true);
  ck_assert_ptr_nonnull(actual);

  const char *const expected =
      "{\n"
      "  \"version\": \"" PACKAGE_VERSION
      "\",\n"
      "  \"max_chain_length\": 64.000000,\n"
      "  \"compression\": false,\n"
      "  \"tables\": {\n"
      "    \"BTL\": {\n"
      "      \"primary_fields\": [\n"
      "        \"first_name\",\n"
      "        \"last_name\",\n"
      "        \"born\"\n"
      "      ],\n"
      "      \"subsidiary_fields\": null\n"
      "    }\n"
      "  },\n"
      "  \"pretty_json\": true\n"
      "}\n";

  ck_assert_str_eq(LCH_BufferData(actual), expected);
  LCH_BufferDestroy(actual);

  LCH_JsonDestroy(config);
}
END_TEST

START_TEST(test_LCH_JsonComposeFile) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonEqual) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonCopy) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonObjectKeysSetMinus) {
  // TODO: Implement
}
END_TEST

START_TEST(test_LCH_JsonObjectKeysSetIntersectAndValuesSetMinus) {
  // TODO: Implement
}
END_TEST

Suite *JSONSuite(void) {
  Suite *s = suite_create("json.c");
  {
    TCase *tc = tcase_create(
        "LCH_Json[Null|True|False|String|Number|Object|Array]Create");
    tcase_add_test(tc, test_LCH_JsonXXXXXCreate);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonGetType");
    tcase_add_test(tc, test_LCH_JsonGetType);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonGetTypeAsString");
    tcase_add_test(tc, test_LCH_JsonGetTypeAsString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc =
        tcase_create("LCH_JsonIs[Null|True|False|String|Number|Array|Object]");
    tcase_add_test(tc, test_LCH_JsonIsXXXXX);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create(
        "LCH_JsonObjectChildIs[Null|True|False|String|Array|Object]");
    tcase_add_test(tc, test_LCH_JsonObjectChildIsXXXXX);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create(
        "LCH_JsonArrayChildIs[Null|True|False|String|Array|Object]");
    tcase_add_test(tc, test_LCH_JsonArrayChildIsXXXXX);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectSet");
    tcase_add_test(tc, test_LCH_JsonObjectSet);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectSetString");
    tcase_add_test(tc, test_LCH_JsonObjectSetString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectSetStringDuplicate");
    tcase_add_test(tc, test_LCH_JsonObjectSetStringDuplicate);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectSetNumber");
    tcase_add_test(tc, test_LCH_JsonObjectSetNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayAppend");
    tcase_add_test(tc, test_LCH_JsonArrayAppend);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayAppendString");
    tcase_add_test(tc, test_LCH_JsonArrayAppendString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonNumberGet");
    tcase_add_test(tc, test_LCH_JsonNumberGet);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectGet");
    tcase_add_test(tc, test_LCH_JsonObjectGet);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayGet");
    tcase_add_test(tc, test_LCH_JsonArrayGet);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonStringGet");
    tcase_add_test(tc, test_LCH_JsonStringGet);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectGetString");
    tcase_add_test(tc, test_LCH_JsonObjectGetString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayGetString");
    tcase_add_test(tc, test_LCH_JsonArrayGetString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectGetObject");
    tcase_add_test(tc, test_LCH_JsonObjectGetObject);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectGetNumber");
    tcase_add_test(tc, test_LCH_JsonObjectGetNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayGetObject");
    tcase_add_test(tc, test_LCH_JsonArrayGetObject);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectGetArray");
    tcase_add_test(tc, test_LCH_JsonObjectGetArray);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectGetKeys");
    tcase_add_test(tc, test_LCH_JsonObjectGetKeys);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectHasKey");
    tcase_add_test(tc, test_LCH_JsonObjectHasKey);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectLength");
    tcase_add_test(tc, test_LCH_JsonObjectLength);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayLength");
    tcase_add_test(tc, test_LCH_JsonArrayLength);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectRemove");
    tcase_add_test(tc, test_LCH_JsonObjectRemove);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayRemove");
    tcase_add_test(tc, test_LCH_JsonArrayRemove);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectRemoveObject");
    tcase_add_test(tc, test_LCH_JsonObjectRemove);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectRemoveArray");
    tcase_add_test(tc, test_LCH_JsonObjectRemove);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonArrayRemoveObject");
    tcase_add_test(tc, test_LCH_JsonArrayRemove);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectRemoveArray");
    tcase_add_test(tc, test_LCH_JsonArrayRemove);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseNull");
    tcase_add_test(tc, test_LCH_JsonParseNull);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseTrue");
    tcase_add_test(tc, test_LCH_JsonParseTrue);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseFalse");
    tcase_add_test(tc, test_LCH_JsonParseFalse);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseNumber");
    tcase_add_test(tc, test_LCH_JsonParseNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseString");
    tcase_add_test(tc, test_LCH_JsonParseString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseArray");
    tcase_add_test(tc, test_LCH_JsonParseArray);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseObject");
    tcase_add_test(tc, test_LCH_JsonParseObject);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParse");
    tcase_add_test(tc, test_LCH_JsonParse);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonParseFile");
    tcase_add_test(tc, test_LCH_JsonParseFile);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeNull");
    tcase_add_test(tc, test_LCH_JsonComposeNull);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeTrue");
    tcase_add_test(tc, test_LCH_JsonComposeTrue);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeFalse");
    tcase_add_test(tc, test_LCH_JsonComposeFalse);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeNumber");
    tcase_add_test(tc, test_LCH_JsonComposeNumber);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeString");
    tcase_add_test(tc, test_LCH_JsonComposeString);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeArray");
    tcase_add_test(tc, test_LCH_JsonComposeArray);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeObject");
    tcase_add_test(tc, test_LCH_JsonComposeObject);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonCompose");
    tcase_add_test(tc, test_LCH_JsonCompose);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonComposeFile");
    tcase_add_test(tc, test_LCH_JsonComposeFile);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonEqual");
    tcase_add_test(tc, test_LCH_JsonEqual);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonCopy");
    tcase_add_test(tc, test_LCH_JsonCopy);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectKeysSetMinus");
    tcase_add_test(tc, test_LCH_JsonObjectKeysSetMinus);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_JsonObjectKeysSetIntersectAndValuesSetMinus");
    tcase_add_test(tc, test_LCH_JsonObjectKeysSetIntersectAndValuesSetMinus);
    suite_add_tcase(s, tc);
  }
  return s;
}
