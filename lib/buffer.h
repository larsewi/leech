#ifndef _LEECH_BUFFER_H
#define _LEECH_BUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "leech.h"

/**
 * Put private LCH_Buffer functions here:
 */

struct LCH_Buffer {
  size_t length;
  size_t capacity;
  char *buffer;
};

static inline LCH_Buffer LCH_BufferStaticFromString(const char *const str) {
  LCH_Buffer buffer;
  buffer.buffer = (char *)str;
  buffer.length = strlen(str);
  buffer.capacity = 0;
  return buffer;
}

/**
 * @brief allocate memory in buffer.
 * @param[in] buffer buffer.
 * @param[in] size number of bytes.
 * @param[out] offset offset to allocated memory in buffer.
 * @return false in case of error.
 */
bool LCH_BufferAllocate(LCH_Buffer *buffer, size_t size, size_t *offset);

/**
 * @brief copy value into buffer at offset.
 * @param[in] buffer buffer.
 * @param[in] value pointer to value.
 * @param[in] size size of value.
 * @param[in] offset offset into buffer.
 */
void LCH_BufferSet(LCH_Buffer *buffer, size_t offset, const void *value,
                   size_t size);

/**
 * @brief Converts bytes into its hexadecimal string representation
 * @param hex Hexadecimal buffer
 * @param bytes Byte buffer
 * @return True on success, false on error
 */
bool LCH_BufferBytesToHex(LCH_Buffer *hex, const LCH_Buffer *bytes);

void LCH_BufferTrim(LCH_Buffer *buffer, char ch);

bool LCH_BufferAppendBuffer(LCH_Buffer *buffer, const LCH_Buffer *append);

/**
 * @brief Checks if buffer contains only printable characters. I.e., character
 *        code 32 - 127 (not included).
 * @param buffer Buffer to check
 * @return True if all characters are printable, false if one or more
 *         characters are non-printable.
 */
bool LCH_BufferIsPrintable(const LCH_Buffer *buffer);

#endif  // _LEECH_BUFFER_H
