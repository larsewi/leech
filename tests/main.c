#include <assert.h>
#include <check.h>
#include <stdio.h>

#include "../lib/leech.h"

static void SetupDebugMessenger(void);

Suite *BufferSuite(void);
Suite *CSVSuite(void);
Suite *DictSuite(void);
Suite *LeechCSVSuite(void);
Suite *ListSuite(void);
Suite *UtilsSuite(void);

int main(int argc, char *argv[]) {
  SetupDebugMessenger();

  SRunner *sr = srunner_create(BufferSuite());
  srunner_add_suite(sr, CSVSuite());
  srunner_add_suite(sr, DictSuite());
  srunner_add_suite(sr, LeechCSVSuite());
  srunner_add_suite(sr, ListSuite());
  srunner_add_suite(sr, UtilsSuite());

  if (argc > 1 && strcmp(argv[1], "no-fork") == 0) {
    srunner_set_fork_status(sr, CK_NOFORK);
  }

  srunner_run_all(sr, CK_NORMAL);
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
                  LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT,
      .messageCallback = &DebugMessengerCallbackDefault,
  };
  LCH_DebugMessengerInit(&initInfo);
}
