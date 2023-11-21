#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include "buffer.h"
#include "leech.h"

LCH_List *LCH_CSVParseTable(const char *str);

LCH_List *LCH_CSVParseRecord(const char *str);

char *LCH_CSVParseField(const char *str);

LCH_List *LCH_CSVParseFile(const char *path);

/****************************************************************************/

/**
 * Compose a CSV formatted string from a table (two dimentional list)
 * @param buffer Buffer to hold the resulting string
 * @param table two dimentional list containing table
 * @note if `buffer` is `NULL`, then a new buffer is created
 * @return `true` on success, else `false`
 */
bool LCH_CSVComposeTable(LCH_Buffer **buffer, const LCH_List *table);

/**
 * Compose a CSV formatted string from a record (one dimentional list)
 * @param buffer Buffer to hold the resulting string
 * @param table one dimentional list containing table
 * @note if `buffer` is `NULL`, then a new buffer is created
 * @return `true` on success, else `false`
 */
bool LCH_CSVComposeRecord(LCH_Buffer **buffer, const LCH_List *record);

char *LCH_CSVComposeField(const char *const field);

bool LCH_CSVComposeFile(const LCH_List *table, const char *path);

#endif  // _LEECH_CSV_H
