#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug_messenger.h"
#include "definitions.h"

#define LCH_COLOR_RED "\x1b[31m"
#define LCH_COLOR_YELLOW "\x1b[33m"
#define LCH_COLOR_GREEN "\x1b[32m"
#define LCH_COLOR_CYAN "\x1b[36m"
#define LCH_COLOR_BLUE "\x1b[34m"
#define LCH_COLOR_RESET "\x1b[0m"

struct LCH_DebugMessenger {
  unsigned char severity;
  void (*messageCallback)(unsigned char, const char *);
};

static struct LCH_DebugMessenger DEBUG_MESSENGER = {
    .severity = 0,
    .messageCallback = NULL,
};

void LCH_DebugMessengerInit(const LCH_DebugMessengerInitInfo *const initInfo) {
  DEBUG_MESSENGER.severity = initInfo->severity;
  DEBUG_MESSENGER.messageCallback = initInfo->messageCallback;
}

void LCH_LogMessage(unsigned char severity, const char *format, ...) {
  if ((DEBUG_MESSENGER.severity & severity) == 0 ||
      DEBUG_MESSENGER.messageCallback == NULL) {
    return;
  }

  va_list ap;
  va_start(ap, format);
  char message[LCH_BUFFER_SIZE];
  int size = vsnprintf(message, sizeof(message), format, ap);
  if (size >= LCH_BUFFER_SIZE) {
    LCH_LOG_WARNING("Log message trucated: Too long (%d >= %d)", size,
                    LCH_BUFFER_SIZE);
  }
  va_end(ap);

  DEBUG_MESSENGER.messageCallback(severity, message);
}

void LCH_DebugMessengerCallbackDefault(unsigned char severity,
                                       const char *message) {
  assert(message != NULL);
  int rc = 0;
  switch (severity) {
  case LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT:
    rc = fprintf(stdout, LCH_COLOR_BLUE "D" LCH_COLOR_RESET ": %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT:
    rc = fprintf(stdout, LCH_COLOR_CYAN "V" LCH_COLOR_RESET ": %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_INFO_BIT:
    rc = fprintf(stdout, LCH_COLOR_GREEN "I" LCH_COLOR_RESET ": %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT:
    rc =
        fprintf(stdout, LCH_COLOR_YELLOW "W" LCH_COLOR_RESET ": %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT:
    rc = fprintf(stderr, LCH_COLOR_RED "E" LCH_COLOR_RESET ": %s\n", message);
    break;
  default:
    break;
  }
  assert(rc >= 0);
}
