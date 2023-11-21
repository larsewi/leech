#include <check.h>
#include <limits.h>

#include "../lib/definitions.h"
#include "../lib/delta.h"
#include "../lib/leech.h"
#include "../lib/leech_csv.h"
#include "../lib/table.h"

START_TEST(test_LCH_Delta) {
  LCH_Buffer *out_buf = LCH_BufferCreate();
  ck_assert_ptr_nonnull(out_buf);

  LCH_TableCreateInfo table_create_info = {
      .identifier = "beatles",
      .primary_fields = "firstname,lastname",
      .subsidiary_fields = "year",
      .read_locator = "test",
      .write_locator = "test",
      .read_callback = LCH_TableReadCallbackCSV,
      .write_callback = LCH_TableWriteCallbackCSV,
      .insert_callback = LCH_TableInsertCallbackCSV,
      .delete_callback = LCH_TableDeleteCallbackCSV,
      .update_callback = LCH_TableUpdateCallbackCSV,
  };
  LCH_Table *beatles = LCH_TableCreate(&table_create_info);
  ck_assert_ptr_nonnull(beatles);

  table_create_info.identifier = "pinkfloyd";
  table_create_info.primary_fields = "id";
  table_create_info.subsidiary_fields = "firstname,lastname";
  LCH_Table *pinkfloyd = LCH_TableCreate(&table_create_info);
  ck_assert_ptr_nonnull(pinkfloyd);

  LCH_InstanceCreateInfo instance_create_info = {
      .work_dir = ".leech",
  };

  LCH_Instance *instance = LCH_InstanceCreate(&instance_create_info);
  ck_assert_ptr_nonnull(instance);

  ck_assert(LCH_InstanceAddTable(instance, beatles));
  ck_assert(LCH_InstanceAddTable(instance, pinkfloyd));

  LCH_Dict *old_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(old_state);
  ck_assert(LCH_DictSet(old_state, "Paul,McCartney", "1942", NULL));
  ck_assert(LCH_DictSet(old_state, "Ringo,Starr", "1940", NULL));
  ck_assert(LCH_DictSet(old_state, "John,Lennon", "1940", NULL));

  LCH_Dict *new_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(new_state);
  ck_assert(LCH_DictSet(new_state, "Paul,McCartney", "1942", NULL));
  ck_assert(LCH_DictSet(new_state, "Ringo,Starr", "1941", NULL));
  ck_assert(LCH_DictSet(new_state, "George,Harrison", "1943", NULL));

  LCH_Delta *delta = LCH_DeltaCreate(beatles, new_state, old_state);
  ck_assert_ptr_nonnull(delta);
  LCH_DictDestroy(old_state);
  LCH_DictDestroy(new_state);

  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);

  ck_assert(LCH_DeltaMarshal(out_buf, delta));
  LCH_DeltaDestroy(delta);

  old_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(old_state);
  ck_assert(LCH_DictSet(old_state, "0", "Syd,Barret", NULL));
  ck_assert(LCH_DictSet(old_state, "1", "Nick,Mason", NULL));
  ck_assert(LCH_DictSet(old_state, "3", "Richard,Wright", NULL));
  ck_assert(LCH_DictSet(old_state, "4", "David,Gilmour", NULL));

  new_state = LCH_DictCreate();
  ck_assert_ptr_nonnull(new_state);
  ck_assert(LCH_DictSet(new_state, "0", "Sid,Barretino", NULL));
  ck_assert(LCH_DictSet(new_state, "2", "Roger,Waters", NULL));
  ck_assert(LCH_DictSet(new_state, "3", "Richard,Wright", NULL));

  delta = LCH_DeltaCreate(pinkfloyd, new_state, old_state);
  ck_assert_ptr_nonnull(delta);
  LCH_DictDestroy(old_state);
  LCH_DictDestroy(new_state);

  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 2);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);

  ck_assert(LCH_DeltaMarshal(out_buf, delta));
  LCH_DeltaDestroy(delta);

  const char *in_buf = LCH_BufferGet(out_buf, 0);

  in_buf = LCH_DeltaUnmarshal(&delta, instance, in_buf);
  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);
  ck_assert_str_eq(LCH_TableGetIdentifier(LCH_DeltaGetTable(delta)), "beatles");
  LCH_DeltaDestroy(delta);

  in_buf = LCH_DeltaUnmarshal(&delta, instance, in_buf);
  ck_assert_int_eq(LCH_DeltaGetNumInsertions(delta), 1);
  ck_assert_int_eq(LCH_DeltaGetNumDeletions(delta), 2);
  ck_assert_int_eq(LCH_DeltaGetNumUpdates(delta), 1);
  ck_assert_str_eq(LCH_TableGetIdentifier(LCH_DeltaGetTable(delta)),
                   "pinkfloyd");
  LCH_DeltaDestroy(delta);

  LCH_InstanceDestroy(instance);
  LCH_BufferDestroy(out_buf);
}
END_TEST

Suite *DeltaSuite(void) {
  Suite *s = suite_create("delta.c");
  {
    TCase *tc = tcase_create("LCH_Delta");
    tcase_add_test(tc, test_LCH_Delta);
    suite_add_tcase(s, tc);
  }
  return s;
}
