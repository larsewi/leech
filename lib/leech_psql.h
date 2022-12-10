#ifndef _LEECH_LEECH_PSQL_H
#define _LEECH_LEECH_PSQL_H

#include "leech.h"

LCH_List *LCH_TableReadCallbackPSQL(const char *const locator);

bool LCH_TableWriteCallbackPSQL(const char *const locator,
                                const LCH_List *const table);

#endif  // _LEECH_LEECH_PSQL_H
