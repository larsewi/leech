#ifndef _LEECH_STRING_LIB_H
#define _LEECH_STRING_LIB_H

#include <stdbool.h>

#include "list.h"

/**
 * Check if two strings are equivalent.
 * @param first first string to check.
 * @param second second string to check.
 * @return true if strings are equivalent, else false.
 */
bool LCH_StringEqual(const char *str1, const char *str2);

/**
 * @brief Split string into a list based on delimiters.
 * @param[in] str String to split.
 * @param[in] del Delimiters to split on.
 * @return List of substrings split on delimiter.
 */
LCH_List *LCH_StringSplit(const char *str, const char *del);

char *LCH_StringJoin(const LCH_List *list, const char *del);

bool LCH_StringStartsWith(const char *str, const char *substr);

char *LCH_StringStrip(char *str, const char *charset);

char *LCH_StringTruncate(const char *str, size_t len, size_t max);

char *LCH_StringDuplicate(const char *str);

char *LCH_StringFormat(const char *format, ...);

bool LCH_StringParseNumber(const char *str, long *number);

bool LCH_StringParseVersion(const char *str, size_t *major, size_t *minor,
                            size_t *patch);

#endif  // _LEECH_STRING_LIB_H