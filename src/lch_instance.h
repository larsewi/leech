#ifndef _LCH_INSTANCE_H
#define _LCH_INSTANCE_H

#include "lch_debug_messenger.h"

#define LCH_LOG_DEBUG(instance, ...)                                           \
  LCH_InstanceLogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT,           \
                         __VA_ARGS__)
#define LCH_LOG_VERBOSE(instance, ...)                                         \
  LCH_InstanceLogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT,         \
                         __VA_ARGS__)
#define LCH_LOG_INFO(instance, ...)                                            \
  LCH_InstanceLogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LCH_LOG_WARNING(instance, ...)                                         \
  LCH_InstanceLogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT,         \
                         __VA_ARGS__)
#define LCH_LOG_ERROR(instance, ...)                                           \
  LCH_InstanceLogMessage(instance, LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT,           \
                         __VA_ARGS__)

typedef struct LCH_Instance LCH_Instance;

typedef struct LCH_InstanceCreateInfo {
  char *instanceID;
  LCH_DebugMessenger *debugMessenger;
} LCH_InstanceCreateInfo;

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo);
void LCH_InstanceDestroy(LCH_Instance **instance);

void LCH_InstanceLogMessage(const LCH_Instance *instance,
                            unsigned char severity, const char *format, ...);

#endif // _LCH_INSTANCE_H
