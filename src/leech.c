#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leech.h"

#define LCH_BUFFER_SIZE 4096UL

#define LCH_COLOR_RED "\x1b[31m"
#define LCH_COLOR_YELLOW "\x1b[33m"
#define LCH_COLOR_GREEN "\x1b[32m"
#define LCH_COLOR_CYAN "\x1b[36m"
#define LCH_COLOR_BLUE "\x1b[34m"
#define LCH_COLOR_RESET "\x1b[0m"

#define LCH_LOG_DEBUG(instance, ...)                                           \
  LCH_LogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT, __VA_ARGS__)
#define LCH_LOG_VERBOSE(instance, ...)                                         \
  LCH_LogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT, __VA_ARGS__)
#define LCH_LOG_INFO(instance, ...)                                            \
  LCH_LogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LCH_LOG_WARNING(instance, ...)                                         \
  LCH_LogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT, __VA_ARGS__)
#define LCH_LOG_ERROR(instance, ...)                                           \
  LCH_LogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT, __VA_ARGS__)

struct LCH_DebugMessenger {
  unsigned char severity;
  void (*callback)(unsigned char, const char *);
};

struct LCH_Instance {
  char *instanceID;
  LCH_DebugMessenger *debugMessenger;
};

static void LCH_LogMessage(const LCH_Instance *instance, unsigned char severity,
                           const char *format, ...);

LCH_DebugMessenger *
LCH_DebugMessengerCreate(const LCH_DebugMessengerCreateInfo *const createInfo) {
  LCH_DebugMessenger *debugMessenger = malloc(sizeof(LCH_DebugMessenger));
  debugMessenger->severity = createInfo->severity;
  debugMessenger->callback = createInfo->callback;
  return debugMessenger;
}

void LCH_DebugMessengerDestroy(LCH_DebugMessenger *debugMessenger) {
  free(debugMessenger);
}

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo) {
  LCH_Instance *instance = malloc(sizeof(LCH_Instance));
  instance->instanceID = strdup(createInfo->instanceID);
  instance->debugMessenger = createInfo->debugMessenger;
  return instance;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  free(instance->instanceID);
  free(instance);
}

void LCH_TestFunc(const LCH_Instance *instance) {
  char *message = "This is a %s message";
  LCH_LOG_DEBUG(instance, message, "debug");
  LCH_LOG_VERBOSE(instance, message, "verbose");
  LCH_LOG_INFO(instance, message, "info");
  LCH_LOG_WARNING(instance, message, "warning");
  LCH_LOG_ERROR(instance, message, "error");
}

void LCH_DebugMessengerCallback(unsigned char severity, const char *message) {
  assert(message != NULL);
  int rc = 0;
  switch (severity) {
  case LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT:
    rc = fprintf(stdout, "[" LCH_COLOR_BLUE "DBUG" LCH_COLOR_RESET "]: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT:
    rc = fprintf(stdout, "[" LCH_COLOR_CYAN "VERB" LCH_COLOR_RESET "]: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_INFO_BIT:
    rc = fprintf(stdout, "[" LCH_COLOR_GREEN "INFO" LCH_COLOR_RESET "]: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT:
    rc =
        fprintf(stdout, "[" LCH_COLOR_YELLOW "WARN" LCH_COLOR_RESET "]: %s\n", message);
    break;
  case LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT:
    rc = fprintf(stderr, "[" LCH_COLOR_RED "ERRR" LCH_COLOR_RESET "]: %s\n", message);
    break;
  default:
    break;
  }
  assert(rc >= 0);
}

static void LCH_LogMessage(const LCH_Instance *instance, unsigned char severity,
                           const char *format, ...) {
  assert(instance != NULL);
  if (instance->debugMessenger == NULL ||
      (instance->debugMessenger->severity & severity) == 0) {
    return;
  }
  assert(instance->debugMessenger->callback != NULL);

  va_list ap;
  va_start(ap, format);
  char message[LCH_BUFFER_SIZE];
  int size = vsnprintf(message, sizeof(message), format, ap);
  assert(size >= 0);
  va_end(ap);

  instance->debugMessenger->callback(severity, message);
}
