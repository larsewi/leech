#ifndef _LEECH_LEECH_CSV_H
#define _LEECH_LEECH_CSV_H

#include <stdbool.h>

#include "dict.h"
#include "leech.h"

LCH_List *LCH_TableReadCallbackCSV(const void *const locator);

bool LCH_TableWriteCallbackCSV(const void *const locator,
                               const LCH_List *const table);

bool LCH_TableInsertCallbackCSV(const void *locator, const char *primary,
                                const char *subsidiary, const LCH_Dict *insert);

bool LCH_TableDeleteCallbackCSV(const void *locator, const char *primary,
                                const char *subsidiary, const LCH_Dict *delete);

bool LCH_TableUpdateCallbackCSV(const void *locator, const char *primary,
                                const char *subsidiary, const LCH_Dict *update);

#endif  // _LEECH_LEECH_CSV_H
