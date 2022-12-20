#ifndef _LEECH_LEECH_CSV_H
#define _LEECH_LEECH_CSV_H

#include <stdbool.h>

#include "leech.h"

LCH_List *LCH_TableReadCallbackCSV(const void *const locator);

bool LCH_TableWriteCallbackCSV(const void *const locator,
                               const LCH_List *const table);

#endif  // _LEECH_LEECH_CSV_H
