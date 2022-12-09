#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include "buffer.h"
#include "list.h"

/* - A field is delimited by a comma character.
 * - A field is escaped if surrounded by double quote characters.
 * - A row is delimited by carriage return line feed sequence.
 * - Fields that contain characters outside of %x20-21 / %x23-2B / %x2D-7E
 *   regions MUST be escaped.
 * - Each instance of a double quote character must be escaped with an
 *   immediately preceding double quote character.
 * - Leading and trailing spaces and tabs are ignored from non-escaped fields.
 * - The final field in a record shall not be escaped by a comma. This would be
 *   interpreted as an additional empty field in the record
 * - The final line need not contain a newline sequence. This would NOT be
 *   interpreted as an additional empty record */

LCH_List *LCH_ParseCSV(const char *str);

LCH_Buffer *LCH_ComposeCSV(const LCH_List *table);

#endif  // _LEECH_CSV_H
