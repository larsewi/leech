#ifndef _LEECH_H
#define _LEECH_H

#include <assert.h>

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT    (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT  (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT     (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT  (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT    (1 << 4)

#define LCH_COLOR_RED     "\x1b[31m"
#define LCH_COLOR_GREEN   "\x1b[32m"
#define LCH_COLOR_YELLOW  "\x1b[33m"
#define LCH_COLOR_BLUE    "\x1b[34m"
#define LCH_COLOR_CYAN    "\x1b[36m"
#define LCH_COLOR_RESET   "\x1b[0m"


typedef struct LCH_DebugMessenger
{
    unsigned char severity;
    void (*callback)(unsigned char, const char *);
} LCH_DebugMessenger;

typedef struct LCH_Instance
{
    LCH_DebugMessenger *debugMessenger;
} LCH_Instance;


void LCH_DebugMessengerCallback(unsigned char severity, const char *message);
void LCH_TestFunc(const LCH_Instance *instance);

#endif // _LEECH_H
