#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>
#include <stdio.h>

#include "buffer.h"
#include "json.h"
#include "list.h"

LCH_Json *LCH_TableToJsonObject(const LCH_List *table,
                                const LCH_List *primary_fields,
                                const LCH_List *subsidiary_fields);

bool LCH_MessageDigest(const unsigned char *message, size_t length,
                       LCH_Buffer *digest);

bool LCH_ListAppendBufferDuplicate(LCH_List *list, const LCH_Buffer *buffer);

/**
 * @brief Safely cast double to size_t.
 * @param number value to cast from
 * @param size value to cast to
 * @return True on success, false if value is out of bounds.
 * @note The size parameter remains untouched on error.
 */
bool LCH_DoubleToSize(double number, size_t *size);

/**
 * @brief Returns a printable string from buffer.
 * @param buffer The buffer to generate printable string from
 * @note The returned string must be free'd with free(3)
 * @return If the buffer content is printable the returned string contains the
 *         original content within two double quotes (i.e., ""). However, if
 *         the buffer content is not printable the returned string contains the
 *         hexadecimal representation of the string within two double quotes
 *         preceeded by the letter b (i.e., b""). NULL is returned in case of
 *         memory errors.
 */
char *LCH_BufferToPrintable(const LCH_Buffer *buffer);

#endif  // _LEECH_UTILS_H
