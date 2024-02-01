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

  const char *const head = ".";
  LCH_Json *const block = LCH_BlockCreate(head, payload);

  ck_assert(LCH_JsonObjectHasKey(block, "timestamp"));
  LCH_LOG_INFO("timestamp: %f",
               LCH_JsonGetNumber(LCH_JsonObjectGet(block, "timestamp")));

  ck_assert(LCH_JsonObjectHasKey(block, "parent"));
  ck_assert_str_eq(LCH_JsonGetString(LCH_JsonObjectGet(block, "parent")), head);
  LCH_LOG_INFO("parent: %s",
               LCH_JsonGetString(LCH_JsonObjectGet(block, "parent")));

  ck_assert(LCH_JsonObjectHasKey(block, "payload"));

  LCH_JsonDestroy(block);
}
END_TEST

Suite *BlockSuite(void) {
  Suite *s = suite_create("block.c");
  {
    TCase *tc = tcase_create("LCH_BlockCreate");
    tcase_add_test(tc, test_LCH_BlockCreate);
    suite_add_tcase(s, tc);
  }
  return s;
}
