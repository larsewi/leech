#ifndef _LEECH_LEECH_H
#define _LEECH_LEECH_H

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT (1 << 4)

#define LCH_LOG_DEBUG(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT, __VA_ARGS__)
#define LCH_LOG_VERBOSE(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT, __VA_ARGS__)
#define LCH_LOG_INFO(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LCH_LOG_WARNING(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT, __VA_ARGS__)
#define LCH_LOG_ERROR(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT, __VA_ARGS__)

typedef struct LCH_DebugMessengerInitInfo {
  unsigned char severity;
  void (*messageCallback)(unsigned char, const char *);
} LCH_DebugMessengerInitInfo;

void LCH_DebugMessengerInit(const LCH_DebugMessengerInitInfo *const initInfo);

void LCH_LogMessage(unsigned char severity, const char *format, ...);

void LCH_DebugMessengerCallbackDefault(unsigned char severity,
                                       const char *message);

typedef struct LCH_Instance LCH_Instance;
typedef struct LCH_InstanceCreateInfo {
  const char *instanceID;
  const char *workDir;
} LCH_InstanceCreateInfo;

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo);
void LCH_InstanceDestroy(LCH_Instance *instance);

typedef struct LCH_Table LCH_Table;

typedef struct LCH_TableCreateInfo {
  const char *readLocator;
  LCH_List *(*readCallback)(const char *);
  const char *writeLocator;
  bool (*writeCallback)(const char *, const LCH_List *);
} LCH_TableCreateInfo;

LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo);

void LCH_TableDestroy(LCH_Table *table);

#endif  // _LEECH_LEECH_H
