#ifndef _LEECH_BUFFER_H
#define _LEECH_BUFFER_H

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct LCH_Buffer LCH_Buffer;

/**
 * @brief create a byte buffer.
 * @note buffer is always null terminated and must be freed with
 *       LCH_BufferDestroy.
 * @return byte buffer.
 */
LCH_Buffer *LCH_BufferCreate(void);

/**
 * @brief format- and print string to byte buffer.
 * @note buffer capacity is expanded if need be.
 * @param[in] self buffer.
 * @param[in] format format string.
 * @return false in case of failure.
 */
bool LCH_BufferPrintFormat(LCH_Buffer *self, const char *format, ...);

/**
 * @brief get length of buffer.
 * @param[in] self buffer.
 * @return length of buffer excluding the terminating null byte.
 */
size_t LCH_BufferLength(LCH_Buffer *self);

/**
 * @brief string duplicate buffer.
 * @note returned string must be freed with free(3).
 * @param[in] buffer.
 * @return heap allocated duplicate.
 */
char *LCH_BufferStringDup(LCH_Buffer *self);

/**
 * @brief destroy buffer.
 * @note noop if self is NULL.
 * @param[in] self buffer.
 */
void LCH_BufferDestroy(LCH_Buffer *self);

/**
 * @brief get buffer.
 * @param [in] self buffer.
 * @return get a pointer to the internal buffer.
 */
const char *LCH_BufferGet(LCH_Buffer *self);

/**
 * @brief allocate memory in buffer.
 * @param[in] self buffer.
 * @return pointer to allocated memory.
 */
void *LCH_BufferAllocate(LCH_Buffer *self, size_t size);

#endif  // _LEECH_BUFFER_H
