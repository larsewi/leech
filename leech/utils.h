#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

/**
 * Split a string with delimitor.
 * @param[in] str string to split.
 * @param[in] del delimitor.
 * @param[out] list list of substrings.
 */
LCH_List *LCH_SplitString(const char *str, const char *del);

bool LCH_StringStartsWith(const char *self, const char *substr);

char *LCH_StringStrip(char *self);

#endif  // _LEECH_UTILS_H
