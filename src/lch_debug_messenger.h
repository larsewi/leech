#ifndef _LCH_DEBUG_MESSENGER_H
#define _LCH_DEBUG_MESSENGER_H

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT (1 << 4)

#define LCH_LOG_DEBUG(instance, ...)                                           \
  LCH_DebugMessengerLogMessage(instance->debugMessenger,                       \
                               LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT, __VA_ARGS__)
#define LCH_LOG_VERBOSE(instance, ...)                                         \
  LCH_DebugMessengerLogMessage(instance->debugMessenger,                       \
                               LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT,             \
                               __VA_ARGS__)
#define LCH_LOG_INFO(instance, ...)                                            \
  LCH_DebugMessengerLogMessage(instance->debugMessenger,                       \
                               LCH_DEBUG_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LCH_LOG_WARNING(instance, ...)                                         \
  LCH_DebugMessengerLogMessage(instance->debugMessenger,                       \
                               LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT,             \
                               __VA_ARGS__)
#define LCH_LOG_ERROR(instance, ...)                                           \
  LCH_DebugMessengerLogMessage(instance->debugMessenger,                       \
                               LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT, __VA_ARGS__)

typedef struct LCH_DebugMessenger {
  unsigned char severity;
  void (*callback)(unsigned char, const char *);
} LCH_DebugMessenger;

void LCH_DebugMessengerCallback(unsigned char severity, const char *message);
void LCH_DebugMessengerLogMessage(const LCH_DebugMessenger *debugMessenger,
                                  unsigned char severity, const char *format,
                                  ...);

#endif // _LCH_DEBUG_MESSENGER_H
