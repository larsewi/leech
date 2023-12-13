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
  const char *str = "one";
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
  str = (char *)LCH_BlockGetData(block);
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
  str = (char *)LCH_BlockGetData(block);
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
  str = (char *)LCH_BlockGetData(block);
  ck_assert_int_eq(len, strlen("one") + 1);
  ck_assert_str_eq(str, "one");
  id = LCH_BlockGetParentID(block);
  ck_assert_ptr_nonnull(id);
  free(block);
  free(id);

  ck_assert_int_eq(rmdir("blocks"), 0);
}
END_TEST

START_TEST(test_LCH_BlockCreate) {
  LCH_Json *const payload = LCH_JsonParse(
      "["
      "  {"
      "    \"type\": \"delta\","
      "    \"version\": \"1.0.0\","
      "    \"inserts\": {"
      "      \"Lennon,John\":\"1940\""
      "    },"
      "    \"updates\": {"
      "      \"Starr,Ringo\": \"1941\""
      "    },"
      "    \"deletes\": {"
      "      \"Harrison,George\": \"1943\""
      "    },"
      "    \"id\": \"beatles\""
      "  }"
      "]");

  const char *const head = "99ad5c35ca6c2d3ba9d7a792bd17550a772c0e6a";
  LCH_Json *const block = LCH_BlockCreateV2(head, payload);

  ck_assert(LCH_JsonObjectHasKey(block, "timestamp"));
  LCH_LOG_INFO("timestamp: %f",
               LCH_JsonNumberGet(LCH_JsonObjectGet(block, "timestamp")));

  ck_assert(LCH_JsonObjectHasKey(block, "parent"));
  ck_assert_str_eq(LCH_JsonStringGet(LCH_JsonObjectGet(block, "parent")), head);
  LCH_LOG_INFO("parent: %s",
               LCH_JsonStringGet(LCH_JsonObjectGet(block, "parent")));

  ck_assert(LCH_JsonObjectHasKey(block, "payload"));

  LCH_JsonDestroy(block);
}
END_TEST

Suite *BlockSuite(void) {
  Suite *s = suite_create("block.c");
  {
    TCase *tc = tcase_create("LCH_Block");
    tcase_add_test(tc, test_LCH_Block);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_BlockCreate");
    tcase_add_test(tc, test_LCH_BlockCreate);
    suite_add_tcase(s, tc);
  }
  return s;
}
