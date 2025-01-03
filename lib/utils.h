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
 * @brief Safily cast double to size_t.
 * @param number value to cast from
 * @param size value to cast to
 * @return True on success, false if value is out of bounds.
 * @note The size parameter remains untouched on error.
 */
bool LCH_DoubleToSize(double number, size_t *size);

#endif  // _LEECH_UTILS_H
