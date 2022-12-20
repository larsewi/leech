#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include "buffer.h"
#include "leech.h"

LCH_List *LCH_ParseCSV(const char *str);

LCH_Buffer *LCH_ComposeCSV(const LCH_List *table);

#endif  // _LEECH_CSV_H
