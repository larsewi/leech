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

typedef struct LCH_DebugMessenger {
  unsigned char severity;
  void (*messageCallback)(unsigned char, const char *);
} LCH_DebugMessenger;

typedef struct LCH_Table {
  char *locator;
  void (*readCallback)(char *);
  void (*writeCallback)(char *);
} LCH_Table;

struct LCH_Instance {
  char *instanceID;
  char *workDir;
  LCH_DebugMessenger *debugMessenger;
  LCH_Table **tables;
  size_t numTables;
};

static LCH_DebugMessenger *
LCH_DebugMessengerCreate(const LCH_DebugMessengerCreateInfo *const createInfo);
static void LCH_DebugMessengerDestroy(LCH_DebugMessenger *debugMessenger);

static LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo);
static void LCH_TableDestroy(LCH_Table *table);

static void LCH_LogMessage(const LCH_Instance *instance, unsigned char severity,
                           const char *format, ...);

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo) {
  if (createInfo == NULL || createInfo->instanceID == NULL ||
      createInfo->workDir == NULL) {
    return NULL;
  }

  LCH_Instance *instance = malloc(sizeof(LCH_Instance));
  assert(instance != NULL);
  memset(instance, 0, sizeof(LCH_Instance));

  instance->instanceID = strdup(createInfo->instanceID);
  assert(instance->instanceID != NULL);

  instance->workDir = strdup(createInfo->workDir);
  assert(instance->workDir != NULL);

  return instance;
}

void LCH_InstanceDestroy(LCH_Instance *instance) {
  if (instance == NULL) {
    return;
  }
  free(instance->instanceID);
  free(instance->workDir);
  LCH_DebugMessengerDestroy(instance->debugMessenger);
  for (size_t i = 0; i < instance->numTables; i++) {
    LCH_TableDestroy(instance->tables[i]);
  }
  free(instance);
}

bool LCH_DebugMessengerAdd(LCH_Instance *instance,
                           LCH_DebugMessengerCreateInfo *createInfo) {
  if (instance == NULL || createInfo == NULL) {
    return false;
  }
  instance->debugMessenger = LCH_DebugMessengerCreate(createInfo);
  LCH_LOG_DEBUG(instance, "Added debug messenger");
  return instance->debugMessenger != NULL;
}

bool LCH_TableAdd(LCH_Instance *instance, LCH_TableCreateInfo *createInfo) {
  if (instance == NULL || createInfo == NULL || createInfo->locator == NULL ||
      createInfo->readCallback == NULL || createInfo->writeCallback == NULL) {
    LCH_LOG_ERROR(instance, "%s: Bad input", __func__);
    return false;
  }

  int i = (instance->numTables)++;

  instance->tables =
      realloc(instance->tables, sizeof(LCH_Table *) * instance->numTables);
  assert(instance->tables != NULL);

  instance->tables[i] = malloc(sizeof(LCH_Table));
  assert(instance->tables[i] != NULL);

  instance->tables[i]->locator = strdup(createInfo->locator);
  assert(instance->tables[i]->locator != NULL);

  instance->tables[i]->readCallback = createInfo->readCallback;
  instance->tables[i]->writeCallback = createInfo->writeCallback;

  return true;
}

void LCH_TableReadCallbackCSV(char *filename) {}
void LCH_TableWriteCallbackCSV(char *filename) {}

void LCH_DebugMessengerCallbackDefault(unsigned char severity,
                                       const char *message) {
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

static LCH_DebugMessenger *
LCH_DebugMessengerCreate(const LCH_DebugMessengerCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->messageCallback != NULL);

  LCH_DebugMessenger *debugMessenger = malloc(sizeof(LCH_DebugMessenger));
  assert(debugMessenger != NULL);
  memset(debugMessenger, 0, sizeof(LCH_DebugMessenger));

  debugMessenger->severity = createInfo->severity;
  debugMessenger->messageCallback = createInfo->messageCallback;
  return debugMessenger;
}

static void LCH_DebugMessengerDestroy(LCH_DebugMessenger *debugMessenger) {
  free(debugMessenger);
}

static LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo) {
  assert(createInfo != NULL);

  LCH_Table *table = malloc(sizeof(LCH_Table));
  assert(table != NULL);
  memset(table, 0, sizeof(LCH_Table));

  table->locator = strdup(createInfo->locator);
  assert(table->locator != NULL);

  table->readCallback = createInfo->readCallback;
  assert(table->readCallback != NULL);

  table->writeCallback = createInfo->writeCallback;
  assert(table->writeCallback != NULL);

  return table;
}

static void LCH_TableDestroy(LCH_Table *table) {
  if (table == NULL) {
    return;
  }
  free(table->locator);
  free(table);
}

static void LCH_LogMessage(const LCH_Instance *instance, unsigned char severity,
                           const char *format, ...) {
  assert(instance != NULL);
  if (instance->debugMessenger == NULL ||
      (instance->debugMessenger->severity & severity) == 0) {
    return;
  }
  assert(instance->debugMessenger->messageCallback != NULL);

  va_list ap;
  va_start(ap, format);
  char message[LCH_BUFFER_SIZE];
  int size = vsnprintf(message, sizeof(message), format, ap);
  assert(size >= 0);
  va_end(ap);

  instance->debugMessenger->messageCallback(severity, message);
}
