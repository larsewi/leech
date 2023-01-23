#include <assert.h>
#include <check.h>
#include <stdio.h>

#include "../lib/leech.h"

static void SetupDebugMessenger(void);

Suite *BlockSuite(void);
Suite *BufferSuite(void);
Suite *CSVSuite(void);
Suite *DeltaSuite(void);
Suite *DictSuite(void);
Suite *HeadSuite(void);
Suite *LeechCSVSuite(void);
Suite *ListSuite(void);
Suite *TableSuite(void);
Suite *UtilsSuite(void);

int main(int argc, char *argv[]) {
  SetupDebugMessenger();

  SRunner *sr = srunner_create(DictSuite());
  srunner_add_suite(sr, BufferSuite());
  srunner_add_suite(sr, ListSuite());
  srunner_add_suite(sr, UtilsSuite());
  srunner_add_suite(sr, DeltaSuite());
  srunner_add_suite(sr, BlockSuite());
  srunner_add_suite(sr, CSVSuite());
  srunner_add_suite(sr, LeechCSVSuite());
  srunner_add_suite(sr, TableSuite());

  if (argc > 1 && strcmp(argv[1], "no-fork") == 0) {
    srunner_set_fork_status(sr, CK_NOFORK);
  }

  srunner_run_all(sr, CK_VERBOSE);
  int number_failed = srunner_ntests_failed(sr);

  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void DebugMessengerCallbackDefault(unsigned char severity,
                                          const char *message) {
  assert(message != NULL);
  switch (severity) {
    case LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT:
      fprintf(stderr, "DEBUG: %s\n", message);
      break;
    case LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT:
      fprintf(stderr, "VERBOSE: %s\n", message);
      break;
    case LCH_DEBUG_MESSAGE_TYPE_INFO_BIT:
      fprintf(stderr, "INFO: %s\n", message);
      break;
    case LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT:
      fprintf(stderr, "WARNING: %s\n", message);
      break;
    case LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT:
      fprintf(stderr, "ERROR: %s\n", message);
      break;
    default:
      assert(false);
  }
}

static void SetupDebugMessenger(void) {
  LCH_DebugMessengerInitInfo initInfo = {
      .severity = LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT |
                  LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT |
                  LCH_DEBUG_MESSAGE_TYPE_INFO_BIT |
                  LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT |
                  LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT,
      .messageCallback = &DebugMessengerCallbackDefault,
  };
  LCH_DebugMessengerInit(&initInfo);
}
