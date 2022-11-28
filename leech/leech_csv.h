#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include <leech.h>
#include <stdbool.h>

#include "utils.h"

LCH_List *LCH_TableReadCallbackCSV(const char *const locator);

bool LCH_TableWriteCallbackCSV(const char *const locator,
                               const LCH_List *const table);

#endif // _LEECH_CSV_H
