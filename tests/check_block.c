#include <check.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../lib/block.h"
#include "../lib/definitions.h"
#include "../lib/leech.h"
#include "../lib/utils.h"

START_TEST(test_LCH_Block) {
  const char *const work_dir = ".";

  if (!LCH_IsDirectory("blocks")) {
    mkdir("blocks", S_IRWXU | S_IRWXG | S_IRWXO);
  }

  // Store one
  char *str = "one";
  size_t len = strlen(str) + 1;
  LCH_Block *block =
      LCH_BlockCreate("0000000000000000000000000000000000000000", str, len);
  ck_assert_ptr_nonnull(block);
  char *id = LCH_BlockStore(work_dir, block);
  free(block);
  ck_assert_ptr_nonnull(id);

  // Store two
  str = "two";
  len = strlen(str) + 1;
  block = LCH_BlockCreate(id, str, len);
  free(id);
  ck_assert_ptr_nonnull(block);
  id = LCH_BlockStore(work_dir, block);
  free(block);
  ck_assert_ptr_nonnull(id);

  // Store three
  str = "three";
  len = strlen(str) + 1;
  block = LCH_BlockCreate(id, str, len);
  free(id);
  ck_assert_ptr_nonnull(block);
  id = LCH_BlockStore(work_dir, block);
  free(block);
  ck_assert_ptr_nonnull(id);

  // Load three
  block = LCH_BlockLoad(work_dir, id);
  ck_assert_ptr_nonnull(block);
  ck_assert(LCH_BlockRemove(work_dir, id));
  free(id);
  len = LCH_BlockGetDataLength(block);
  str = LCH_BlockGetData(block);
  ck_assert_int_eq(len, strlen("three") + 1);
  ck_assert_str_eq(str, "three");
  id = LCH_BlockGetParentID(block);
  free(block);
  ck_assert_ptr_nonnull(id);

  // Load two
  block = LCH_BlockLoad(work_dir, id);
  ck_assert_ptr_nonnull(block);
  ck_assert(LCH_BlockRemove(work_dir, id));
  free(id);
  len = LCH_BlockGetDataLength(block);
  str = LCH_BlockGetData(block);
  ck_assert_int_eq(len, strlen("two") + 1);
  ck_assert_str_eq(str, "two");
  id = LCH_BlockGetParentID(block);
  free(block);
  ck_assert_ptr_nonnull(id);

  // Load one
  block = LCH_BlockLoad(work_dir, id);
  ck_assert_ptr_nonnull(block);
  ck_assert(LCH_BlockRemove(work_dir, id));
  free(id);
  len = LCH_BlockGetDataLength(block);
  str = LCH_BlockGetData(block);
  ck_assert_int_eq(len, strlen("one") + 1);
  ck_assert_str_eq(str, "one");
  id = LCH_BlockGetParentID(block);
  ck_assert_ptr_nonnull(id);
  free(block);
  free(id);

  ck_assert_int_eq(rmdir("blocks"), 0);
}
END_TEST

Suite *BlockSuite(void) {
  Suite *s = suite_create("block.c");
  TCase *tc = tcase_create("LCH_Block");
  tcase_add_test(tc, test_LCH_Block);
  suite_add_tcase(s, tc);
  return s;
}
