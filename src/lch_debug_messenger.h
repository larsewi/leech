#ifndef _LCH_DEBUG_MESSENGER_H
#define _LCH_DEBUG_MESSENGER_H

#include <stdarg.h>

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT (1 << 4)

typedef struct LCH_DebugMessenger LCH_DebugMessenger;

typedef struct LCH_DebugMessengerCreateInfo {
  unsigned char severity;
  void (*callback)(unsigned char, const char *);
} LCH_DebugMessengerCreateInfo;

LCH_DebugMessenger *
LCH_DebugMessengerCreate(const LCH_DebugMessengerCreateInfo *const createInfo);
void LCH_DebugMessengerDestroy(LCH_DebugMessenger **debugMessenger);

void LCH_DebugMessengerCallback(unsigned char severity, const char *message);
void LCH_DebugMessengerLogMessageV(const LCH_DebugMessenger *debugMessenger,
                                   unsigned char severity, const char *format,
                                   va_list ap);
void LCH_DebugMessengerLogMessage(const LCH_DebugMessenger *debugMessenger,
                                  unsigned char severity, const char *format,
                                  ...);

#endif // _LCH_DEBUG_MESSENGER_H
