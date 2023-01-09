#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include "buffer.h"
#include "leech.h"

LCH_List *LCH_CSVParse(const char *str);

LCH_List *LCH_CSVParseRecord(const char *str);

LCH_List *LCH_CSVParseFile(const char *path);

LCH_Buffer *LCH_CSVCompose(const LCH_List *table);

LCH_Buffer *LCH_CSVComposeRecord(const LCH_List *record);

char *LCH_CSVComposeField(const char *const str);

bool LCH_CSVComposeFile(const LCH_List *table, const char *path);

#endif  // _LEECH_CSV_H
