#ifndef _LEECH_LEECH_H
#define _LEECH_LEECH_H

#include <stdbool.h>
#include <stdlib.h>

/****************************************************************************/
/*  Debug Messenger                                                         */
/****************************************************************************/

#define LCH_LOGGER_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_LOGGER_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_LOGGER_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_LOGGER_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_LOGGER_MESSAGE_TYPE_ERROR_BIT (1 << 4)

typedef void (*LCH_LoggerCallbackFn)(unsigned char, const char *);

void LCH_LoggerInit(unsigned char severity, LCH_LoggerCallbackFn callback);

void LCH_LoggerCallbackDefault(unsigned char severity, const char *message);

bool LCH_Commit(const char *work_dir);

char *LCH_Diff(const char *work_dir, const char *block_id, size_t *buf_len);

char *LCH_Rebase(const char *work_dir, size_t *buf_len);

bool LCH_Patch(const char *work_dir, const char *patch, const char *uid_field,
               const char *uid_value, size_t size);

#endif  // _LEECH_LEECH_H
