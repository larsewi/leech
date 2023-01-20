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
 * @param[in] offset offset into buffer.
 * @return pointer to internal buffer.
 */
const void *LCH_BufferGet(LCH_Buffer *self, size_t offset);

/**
 * @brief allocate memory in buffer.
 * @param[in] self buffer.
 * @param[in] size number of bytes.
 * @param[out] offset offset to allocated memory in buffer.
 * @return false in case of error.
 */
bool LCH_BufferAllocate(LCH_Buffer *self, size_t size, size_t *offset);

/**
 * @brief copy value into buffer at offset.
 * @param[in] self buffer.
 * @param[in] value pointer to value.
 * @param[in] size size of value.
 * @param[in] offset offset into buffer.
 */
void LCH_BufferSet(LCH_Buffer *self, size_t offset, const void *value,
                   size_t size);

#endif  // _LEECH_BUFFER_H
