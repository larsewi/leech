#ifndef _LEECH_UTILS
#define _LEECH_UTILS

#include "leech.h"

/**
 * Split a string with delimitor.
 * @param[in] str string to split.
 * @param[in] del delimitor.
 * @param[out] list list of substrings.
 */
LCH_List *LCH_SplitString(const char *str, const char *del);

bool LCH_StringStartsWith(const char *str, const char *substr);

char *LCH_StringStrip(char *str, const char *charset);

bool LCH_FileSize(FILE *file, size_t *size);

#define LCH_SHA1_LENGTH 40

unsigned char *LCH_SHA1(const void *message, size_t length);

#endif  // _LEECH_UTILS
