#ifndef _LEECH_LOGGER
#define _LEECH_LOGGER

#include "leech.h"

#define LCH_LOG_DEBUG(...) \
  LCH_LoggerLogMessage(LCH_LOGGER_MESSAGE_TYPE_DEBUG_BIT, __VA_ARGS__)
#define LCH_LOG_VERBOSE(...) \
  LCH_LoggerLogMessage(LCH_LOGGER_MESSAGE_TYPE_VERBOSE_BIT, __VA_ARGS__)
#define LCH_LOG_INFO(...) \
  LCH_LoggerLogMessage(LCH_LOGGER_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LCH_LOG_WARNING(...) \
  LCH_LoggerLogMessage(LCH_LOGGER_MESSAGE_TYPE_WARNING_BIT, __VA_ARGS__)
#define LCH_LOG_ERROR(...) \
  LCH_LoggerLogMessage(LCH_LOGGER_MESSAGE_TYPE_ERROR_BIT, __VA_ARGS__)

void LCH_LoggerLogMessage(unsigned char severity, const char *format, ...);

#endif  // _LEECH_LOGGER
