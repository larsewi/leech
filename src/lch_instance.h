#ifndef _LCH_INSTANCE_H
#define _LCH_INSTANCE_H

#include "lch_debug_messenger.h"

#define LCH_LOG_DEBUG(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT, __VA_ARGS__)
#define LCH_LOG_VERBOSE(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT, __VA_ARGS__)
#define LCH_LOG_INFO(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LCH_LOG_WARNING(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT, __VA_ARGS__)
#define LCH_LOG_ERROR(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT, __VA_ARGS__)


typedef struct LCH_Instance
{
    LCH_DebugMessenger *debugMessenger;
} LCH_Instance;

void logDebugMessenger(const LCH_Instance *instance, unsigned char severity, const char *format, ...);

#endif // _LCH_INSTANCE_H
