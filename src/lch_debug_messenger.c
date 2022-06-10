#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lch_debug_messenger.h"
#include "lch_utils.h"

#define LCH_COLOR_RED "\x1b[31m"
#define LCH_COLOR_GREEN "\x1b[32m"
#define LCH_COLOR_YELLOW "\x1b[33m"
#define LCH_COLOR_BLUE "\x1b[34m"
#define LCH_COLOR_CYAN "\x1b[36m"
#define LCH_COLOR_RESET "\x1b[0m"

struct LCH_DebugMessenger {
  unsigned char severity;
  void (*callback)(unsigned char, const char *);
};

LCH_DebugMessenger *
LCH_DebugMessengerCreate(const LCH_DebugMessengerCreateInfo *const createInfo) {
  LCH_DebugMessenger *debugMessenger = malloc(sizeof(LCH_DebugMessenger));
  debugMessenger->severity = createInfo->severity;
  debugMessenger->callback = createInfo->callback;
  return debugMessenger;
}

void LCH_DebugMessengerDestroy(LCH_DebugMessenger **debugMessenger) {
  free(*debugMessenger);
  *debugMessenger = NULL;
}

void LCH_DebugMessengerCallback(unsigned char severity, const char *message) {
  assert(message != NULL);
  int rc = 0;

  switch (severity) {
  case LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT:
    rc = fprintf(stdout, "[" LCH_COLOR_BLUE "DBUG" LCH_COLOR_RESET "]: %s\n",
                 message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT:
    rc = fprintf(stdout, "[" LCH_COLOR_CYAN "VERB" LCH_COLOR_RESET "]: %s\n",
                 message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_INFO_BIT:
    rc = fprintf(stdout, "[" LCH_COLOR_GREEN "INFO" LCH_COLOR_RESET "]: %s\n",
                 message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT:
    rc = fprintf(stdout, "[" LCH_COLOR_YELLOW "WARN" LCH_COLOR_RESET "]: %s\n",
                 message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT:
    rc = fprintf(stderr, "[" LCH_COLOR_RED "ERRR" LCH_COLOR_RESET "]: %s\n",
                 message);
    break;
  default:
    break;
  }

  assert(rc >= 0);
}

void LCH_DebugMessengerLogMessageV(const LCH_DebugMessenger *debugMessenger,
                                   unsigned char severity, const char *format,
                                   va_list ap) {
  if (debugMessenger == NULL || (debugMessenger->severity & severity) == 0) {
    return;
  }
  assert(debugMessenger->callback != NULL);

  char message[LCH_BUFFER_SIZE];
  int size = vsnprintf(message, sizeof(message), format, ap);
  assert(size >= 0);
  if ((unsigned long)size >= sizeof(message)) {
    LCH_DebugMessengerLogMessage(
        debugMessenger, LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT,
        "Debug messenger output truncated (%lu > %lu)", size, sizeof(message));
  }

  debugMessenger->callback(severity, message);
}

void LCH_DebugMessengerLogMessage(const LCH_DebugMessenger *debugMessenger,
                                  unsigned char severity, const char *format,
                                  ...) {
  va_list ap;
  va_start(ap, format);
  LCH_DebugMessengerLogMessageV(debugMessenger, severity, format, ap);
  va_end(ap);
}
