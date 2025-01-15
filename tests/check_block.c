#include <check.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../lib/block.h"
#include "../lib/files.h"
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

START_TEST(test_LCH_BlockIdFromArgument) {
  char template[] = "tmp_XXXXXX";
  const char *work_dir = mkdtemp(template);
  ck_assert_ptr_nonnull(work_dir);

  const char *blocks[] = {
      "0820ee7abd43af0221f2ad3f81f667dd87cad6c8",
      "0957d9468925b66a5acbdd6551c11dc6344337b3",
      "be3e991161dcde612b61be9562e08942e9a47903",
      "f80cc0ac9dc567acb99b076412c9884cbaa84f0",
      "8de90ff4c64c0a251d0cdb45b4ec2253b7c6e2a3a",
      "invalid         block         identifier",
      "3d28755d158bf7a7aabbc308c527fdc9d413c9c8",
      "3d28755d158b158bf7a7aabbc308c527fdc9d4c8",
      NULL,
  };

  LCH_Buffer *const buffer = LCH_BufferCreate();
  ck_assert_ptr_nonnull(buffer);

  char filename[PATH_MAX];
  for (size_t i = 0; blocks[i] != NULL; i++) {
    /* Create some empty files in $(work_dir)/blocks/ */
    ck_assert(
        LCH_FilePathJoin(filename, PATH_MAX, 3, work_dir, "blocks", blocks[i]));
    ck_assert(LCH_BufferWriteFile(buffer, filename));
  }

  LCH_BufferDestroy(buffer);

  char *block_id = LCH_BlockIdFromArgument(work_dir, "0820ee7");
  ck_assert_ptr_nonnull(block_id);
  free(block_id);

  block_id = LCH_BlockIdFromArgument(work_dir, "0957d946");
  ck_assert_ptr_nonnull(block_id);
  free(block_id);

  /* Try with the entire hash */
  block_id = LCH_BlockIdFromArgument(
      work_dir, "be3e991161dcde612b61be9562e08942e9a47903");
  ck_assert_ptr_nonnull(block_id);
  free(block_id);

  /* Try with more than the entire hash */
  block_id = LCH_BlockIdFromArgument(
      work_dir, "be3e991161dcde612b61be9562e08942e9a47903a");
  ck_assert_ptr_null(block_id);
  free(block_id);

  block_id = LCH_BlockIdFromArgument(work_dir, "957d94");
  ck_assert_ptr_null(block_id);
  free(block_id);

  block_id = LCH_BlockIdFromArgument(work_dir, "f80cc0ac9");
  ck_assert_ptr_null(block_id);
  free(block_id);

  block_id = LCH_BlockIdFromArgument(work_dir, "8de90ff4c64c0");
  ck_assert_ptr_null(block_id);
  free(block_id);

  block_id = LCH_BlockIdFromArgument(work_dir, "invalid");
  ck_assert_ptr_null(block_id);
  free(block_id);

  block_id = LCH_BlockIdFromArgument(work_dir, "3d28755d158b");
  ck_assert_ptr_null(block_id);
  free(block_id);

  block_id = LCH_BlockIdFromArgument(work_dir, "3d28755d158b1");
  ck_assert_ptr_nonnull(block_id);
  free(block_id);

  ck_assert(LCH_FileDelete(work_dir));
}
END_TEST

Suite *BlockSuite(void) {
  Suite *s = suite_create("block.c");
  {
    TCase *tc = tcase_create("LCH_BlockCreate");
    tcase_add_test(tc, test_LCH_BlockCreate);
    suite_add_tcase(s, tc);
  }
  {
    TCase *tc = tcase_create("LCH_BlockIdFromArgument");
    tcase_add_test(tc, test_LCH_BlockIdFromArgument);
    suite_add_tcase(s, tc);
  }
  return s;
}
