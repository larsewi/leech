#include <check.h>

#include "../lib/definitions.h"
#include "../lib/json.h"
#include "../lib/patch.h"

START_TEST(test_LCH_PatchGetVersion) {
  const char *const raw = "{ \"version\": 1 }";
  LCH_Json *const patch = LCH_JsonParse(raw, strlen(raw));
  ck_assert_ptr_nonnull(patch);

  size_t version;
  ck_assert(LCH_PatchGetVersion(patch, &version));
  ck_assert_uint_eq(version, 1);

  LCH_JsonDestroy(patch);
}
END_TEST

START_TEST(test_LCH_PatchParse) {
  {
    LCH_Buffer *const buffer = LCH_BufferCreate();
    ck_assert_ptr_nonnull(buffer);
    ck_assert(LCH_BufferPrintFormat(buffer, "{ \"version\": %zu }",
                                    LCH_PATCH_VERSION));

    const size_t raw_length = LCH_BufferLength(buffer);
    char *const raw_buffer = LCH_BufferToString(buffer);

    LCH_Json *const patch = LCH_PatchParse(raw_buffer, raw_length);
    ck_assert_ptr_nonnull(patch);

    LCH_JsonDestroy(patch);
    free(raw_buffer);
  }
  {
    LCH_Buffer *const buffer = LCH_BufferCreate();
    ck_assert_ptr_nonnull(buffer);
    ck_assert(LCH_BufferPrintFormat(buffer, "{ \"version\": %zu }",
                                    LCH_PATCH_VERSION + 1));

    const size_t raw_length = LCH_BufferLength(buffer);
    char *const raw_buffer = LCH_BufferToString(buffer);

    LCH_Json *const patch = LCH_PatchParse(raw_buffer, raw_length);
    ck_assert_ptr_null(patch);
    free(raw_buffer);
  }
}
END_TEST

Suite *PatchSuite(void) {
  Suite *s = suite_create("list.c");
  {
    TCase *tc = tcase_create("LCH_PatchGetVersion");
    tcase_add_test(tc, test_LCH_PatchGetVersion);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_PatchParse");
    tcase_add_test(tc, test_LCH_PatchParse);
    suite_add_tcase(s, tc);
  }
  return s;
}
