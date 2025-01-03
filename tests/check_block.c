#include <check.h>

#include "../lib/block.h"
#include "../lib/logger.h"

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

  {
    const LCH_Buffer key = LCH_BufferStaticFromString("timestamp");
    ck_assert(LCH_JsonObjectHasKey(block, &key));
  }
  {
    const LCH_Buffer key = LCH_BufferStaticFromString("parent");
    ck_assert(LCH_JsonObjectHasKey(block, &key));
  }
  {
    const LCH_Buffer key = LCH_BufferStaticFromString("payload");
    ck_assert(LCH_JsonObjectHasKey(block, &key));
  }

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
