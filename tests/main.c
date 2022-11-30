#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <check.h>

#include "../leech/debug_messenger.h"

static void SetupDebugMessenger(void);

Suite *BufferSuite(void);
Suite *CSVSuite(void);
Suite *DictSuite(void);
Suite *ListSuite(void);
Suite *UtilsSuite(void);

int main(void) {
  SetupDebugMessenger();

  SRunner *sr = srunner_create(BufferSuite());
  srunner_add_suite(sr, CSVSuite());
  srunner_add_suite(sr, DictSuite());
  srunner_add_suite(sr, ListSuite());
  srunner_add_suite(sr, UtilsSuite());

  srunner_run_all(sr, CK_VERBOSE );
  int number_failed = srunner_ntests_failed(sr);

  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void DebugMessengerCallbackDefault(unsigned char severity,
                                       const char *message) {
  assert(message != NULL);
  switch (severity) {
  case LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT:
    printf("DEBUG: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT:
    printf("VERBOSE: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_INFO_BIT:
    printf("INFO: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT:
    printf("WARNING: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT:
    printf("ERROR: %s\n", message);
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
