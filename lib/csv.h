#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include "buffer.h"
#include "leech.h"

LCH_List *LCH_CSVParseTable(const char *str);

LCH_List *LCH_CSVParseRecord(const char *str);

char *LCH_CSVParseField(const char *str);

LCH_List *LCH_CSVParseFile(const char *path);

/****************************************************************************/

bool LCH_CSVComposeTable(LCH_Buffer **buffer, const LCH_List *table);

bool LCH_CSVComposeRecord(LCH_Buffer **buffer, const LCH_List *record);

char *LCH_CSVComposeField(const char *const field);

bool LCH_CSVComposeFile(const LCH_List *table, const char *path);

#endif  // _LEECH_CSV_H
