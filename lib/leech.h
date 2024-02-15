#ifndef _LEECH_LEECH_H
#define _LEECH_LEECH_H

#include <stdbool.h>
#include <stdlib.h>

/****************************************************************************/
/*  Debug Messenger                                                         */
/****************************************************************************/

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

bool LCH_Commit(const char *work_dir);

char *LCH_Diff(const char *work_dir, const char *block_id, size_t *buf_len);

bool LCH_Patch(const char *work_dir, const char *patch, const char *uid_field,
               const char *uid_value, size_t size);

#endif  // _LEECH_LEECH_H
