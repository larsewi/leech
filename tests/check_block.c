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
  const char *const csv =
      "["
      "  {"
      "    \"type\": \"delta\","
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
      "]";
  LCH_Json *const payload = LCH_JsonParse(csv, strlen(csv));

  const char *const head = "I'm the parent";
  LCH_Json *const block = LCH_BlockCreate(head, payload);

  ck_assert(
      LCH_JsonObjectHasKey(block, LCH_BufferStaticFromString("timestamp")));
  LCH_LOG_INFO("timestamp: %f",
               LCH_JsonNumberGet(LCH_JsonObjectGet(
                   block, LCH_BufferStaticFromString("timestamp"))));

  ck_assert(LCH_JsonObjectHasKey(block, LCH_BufferStaticFromString("parent")));
  ck_assert_str_eq(LCH_BufferData(LCH_JsonObjectGetString(
                       block, LCH_BufferStaticFromString("parent"))),
                   head);
  LCH_LOG_INFO("parent: %s", LCH_JsonStringGet(LCH_JsonObjectGet(
                                 block, LCH_BufferStaticFromString("parent"))));

  ck_assert(LCH_JsonObjectHasKey(block, LCH_BufferStaticFromString("payload")));

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
