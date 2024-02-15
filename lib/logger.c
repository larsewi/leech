#include "logger.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "definitions.h"
#include "leech.h"

#ifdef LCH_ENABLE_COLOR
#define LCH_COLOR_RED "\x1b[31m"
#define LCH_COLOR_YELLOW "\x1b[33m"
#define LCH_COLOR_GREEN "\x1b[32m"
#define LCH_COLOR_CYAN "\x1b[36m"
#define LCH_COLOR_BLUE "\x1b[34m"
#define LCH_COLOR_RESET "\x1b[0m"
#else
#define LCH_COLOR_RED ""
#define LCH_COLOR_YELLOW ""
#define LCH_COLOR_GREEN ""
#define LCH_COLOR_CYAN ""
#define LCH_COLOR_BLUE ""
#define LCH_COLOR_RESET ""
#endif

struct Logger {
  unsigned char severity;
  void (*messageCallback)(unsigned char, const char *);
};

static struct Logger LOGGER = {.severity = 0, .messageCallback = NULL};

void LCH_LoggerInit(unsigned char level, LCH_LoggerCallbackFn callback) {
  LOGGER.severity = level;
  LOGGER.messageCallback = callback;
}

void LCH_LoggerLogMessage(unsigned char severity, const char *format, ...) {
  if ((LOGGER.severity & severity) == 0 || LOGGER.messageCallback == NULL) {
    return;
  }

  va_list ap;
  va_start(ap, format);
  char message[LCH_BUFFER_SIZE];
  int size = vsnprintf(message, sizeof(message), format, ap);
  if (size < 0 || (size_t)size >= LCH_BUFFER_SIZE) {
    LCH_LOG_WARNING("Log message trucated: Too long (%d >= %d)", size,
                    LCH_BUFFER_SIZE);
  }
  va_end(ap);

  LOGGER.messageCallback(severity, message);
}

void LCH_LoggerCallbackDefault(unsigned char severity, const char *message) {
  assert(message != NULL);
  switch (severity) {
    case LCH_LOGGER_MESSAGE_TYPE_DEBUG_BIT:
      fprintf(stdout, LCH_COLOR_BLUE "  DEBUG" LCH_COLOR_RESET ": %s\n",
              message);
      break;
    case LCH_LOGGER_MESSAGE_TYPE_VERBOSE_BIT:
      fprintf(stdout, LCH_COLOR_CYAN "VERBOSE" LCH_COLOR_RESET ": %s\n",
              message);
      break;
    case LCH_LOGGER_MESSAGE_TYPE_INFO_BIT:
      fprintf(stdout, LCH_COLOR_GREEN "   INFO" LCH_COLOR_RESET ": %s\n",
              message);
      break;
    case LCH_LOGGER_MESSAGE_TYPE_WARNING_BIT:
      fprintf(stdout, LCH_COLOR_YELLOW "WARNING" LCH_COLOR_RESET ": %s\n",
              message);
      break;
    case LCH_LOGGER_MESSAGE_TYPE_ERROR_BIT:
      fprintf(stderr, LCH_COLOR_RED "  ERROR" LCH_COLOR_RESET ": %s\n",
              message);
      break;
    default:
      break;
  }
}
