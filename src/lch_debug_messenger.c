#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "lch_debug_messenger.h"
#include "lch_utils.h"

#define LCH_COLOR_RED "\x1b[31m"
#define LCH_COLOR_GREEN "\x1b[32m"
#define LCH_COLOR_YELLOW "\x1b[33m"
#define LCH_COLOR_BLUE "\x1b[34m"
#define LCH_COLOR_CYAN "\x1b[36m"
#define LCH_COLOR_RESET "\x1b[0m"

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

void LCH_DebugMessengerLogMessage(const LCH_DebugMessenger *debugMessenger,
                                  unsigned char severity, const char *format,
                                  ...) {
  if (debugMessenger == NULL) {
    return;
  }
  if ((debugMessenger->severity & severity) == 0) {
    return;
  }
  assert(debugMessenger->callback != NULL);

  va_list ap;
  va_start(ap, format);

  char message[LCH_BUFFER_SIZE];
  int size = vsnprintf(message, sizeof(message), format, ap);
  assert(size >= 0);

  va_end(ap);

  if ((unsigned long)size >= sizeof(message)) {
    LCH_DebugMessengerLogMessage(
        debugMessenger, LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT,
        "Debug messenger output truncated (%lu > %lu)", size, sizeof(message));
  }

  debugMessenger->callback(severity, message);
}
