#ifndef _LEECH_COMMON
#define _LEECH_COMMON

#include <getopt.h>

#include "../lib/leech.h"

void PrintVersion(void);

void PrintOptions(const struct option *const options,
                  const char *const *const descriptions);

#endif  // _LEECH_COMMON
