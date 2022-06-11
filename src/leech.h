#ifndef _LEECH_H
#define _LEECH_H

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

typedef struct LCH_Instance LCH_Instance;
typedef struct LCH_InstanceCreateInfo {
  char *instanceID;
  LCH_DebugMessenger *debugMessenger;
} LCH_InstanceCreateInfo;

LCH_DebugMessenger *
LCH_DebugMessengerCreate(const LCH_DebugMessengerCreateInfo *const createInfo);
void LCH_DebugMessengerDestroy(LCH_DebugMessenger *debugMessenger);

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo);
void LCH_InstanceDestroy(LCH_Instance *instance);

void LCH_DebugMessengerCallback(unsigned char severity, const char *message);

void LCH_TestFunc(const LCH_Instance *instance);

#endif // _LEECH_H
