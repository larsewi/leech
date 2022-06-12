#ifndef _LEECH_H
#define _LEECH_H

#include <stdbool.h>

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT (1 << 4)

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
  char *locator;
  void (*readCallback)(char *);
  void (*writeCallback)(char *);
} LCH_TableCreateInfo;

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo);
void LCH_InstanceDestroy(LCH_Instance *instance);

bool LCH_DebugMessengerAdd(LCH_Instance *instance,
                           LCH_DebugMessengerCreateInfo *createInfo);
bool LCH_TableAdd(LCH_Instance *instance, LCH_TableCreateInfo *createInfo);

void LCH_DebugMessengerCallbackDefault(unsigned char severity,
                                       const char *message);
void LCH_TableReadCallbackCSV(char *filename);
void LCH_TableWriteCallbackCSV(char *filename);

#endif // _LEECH_H
