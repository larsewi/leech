#ifndef _LEECH_BUFFER_H
#define _LEECH_BUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "leech.h"

/**
 * Put private LCH_Buffer functions here:
 */

/**
 * @brief Self expanding byte buffer
 * @note Always null-byte terminated
 */
struct LCH_Buffer {
  size_t length;
  size_t capacity;
  char *buffer;
};

/**
 * @brief Create a buffer from a static string
 * @param str A statically allocated string
 * @return A stack allocated byte buffer
 * @note This function is mainly here to avoid having use the heap, which would
 *       require checking potential allocation errors
 * @warning Don't use the returned buffer on any functions that would mutate the
 *          string passed as the argument
 */
static inline LCH_Buffer LCH_BufferStaticFromString(const char *const str) {
  LCH_Buffer buffer;
  buffer.buffer = (char *)str;
  buffer.length = strlen(str);
  buffer.capacity = 0;
  return buffer;
}

/**
 * @brief Allocate memory in the buffer
 * @param buffer The byte buffer
 * @param size The number of bytes to allocate
 * @param offset Variable to store the offset of allocated memory in the buffer.
 *               This will be the current buffer length before the allocation
 *               takes place
 * @return False in case of failure
 */
bool LCH_BufferAllocate(LCH_Buffer *buffer, size_t size, size_t *offset);

/**
 * @brief Copy a value into the byte buffer at a given offset
 * @param buffer The byte buffer
 * @param value A pointer to the value
 * @param size The size of the value
 * @param offset The offset in the buffer
 */
void LCH_BufferSet(LCH_Buffer *buffer, size_t offset, const void *value,
                   size_t size);

/**
 * @brief Convert bytes into a hexadecimal string representation
 * @param hex The byte buffer to write the hexadecimal string into
 * @param bytes The byte buffer containing the bytes to be converted
 * @return False in case of failure
 */
bool LCH_BufferBytesToHex(LCH_Buffer *hex, const LCH_Buffer *bytes);

/**
 * @brief Trim leading and trailing characters in a byte buffer
 * @param buffer The buffer
 * @param ch The character to trim
 */
void LCH_BufferTrim(LCH_Buffer *buffer, char ch);

/**
 * @brief Append the contents of one buffer onto another buffer
 * @param buffer The buffer to append to
 * @param append The buffer to append from
 * @return False in case of failure
 */
bool LCH_BufferAppendBuffer(LCH_Buffer *buffer, const LCH_Buffer *append);

/**
 * @brief Checks if buffer contains only printable characters. I.e., character
 *        code 32 - 127 (not included)
 * @param buffer Buffer to check
 * @return True if all characters are printable, false if one or more
 *         characters are non-printable
 */
bool LCH_BufferIsPrintable(const LCH_Buffer *buffer);

#endif  // _LEECH_BUFFER_H
