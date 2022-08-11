#ifndef _LEECH_H
#define _LEECH_H

#include <stdbool.h>

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT (1 << 4)

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

typedef struct LCH_Instance LCH_Instance;
typedef struct LCH_InstanceCreateInfo {
  char *instanceID;
  char *workDir;
} LCH_InstanceCreateInfo;

typedef struct LCH_DebugMessengerCreateInfo {
  unsigned char severity;
  void (*messageCallback)(unsigned char, const char *);
} LCH_DebugMessengerCreateInfo;

typedef struct LCH_TableCreateInfo {
  char *readLocator;
  bool (*readCallback)(const LCH_Instance *);
  char *writeLocator;
  bool (*writeCallback)(const LCH_Instance *);
} LCH_TableCreateInfo;

typedef struct LCH_TableHeader {

} LCH_TableHeader;

typedef struct LCH_TableRecord {

} LCH_TableRecord;

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo);
void LCH_InstanceDestroy(LCH_Instance *instance);

bool LCH_DebugMessengerAdd(LCH_Instance *instance,
                           LCH_DebugMessengerCreateInfo *createInfo);
bool LCH_TableAdd(LCH_Instance *instance, LCH_TableCreateInfo *createInfo);

void LCH_DebugMessengerCallbackDefault(unsigned char severity,
                                       const char *message);

void LCH_LogMessage(const LCH_Instance *instance, unsigned char severity,
                    const char *format, ...);

#endif // _LEECH_H
