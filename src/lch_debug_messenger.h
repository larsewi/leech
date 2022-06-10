#ifndef _LCH_DEBUG_MESSENGER_H
#define _LCH_DEBUG_MESSENGER_H

#include "lch_instance.h"

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT    (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT  (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT     (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT  (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT    (1 << 4)


typedef struct LCH_DebugMessenger
{
    unsigned char severity;
    void (*callback)(unsigned char, const char *);
} LCH_DebugMessenger;

void LCH_DebugMessengerCallback(unsigned char severity, const char *message);

#endif // _LCH_DEBUG_MESSENGER_H
