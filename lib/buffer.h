#ifndef _LEECH_BUFFER_H
#define _LEECH_BUFFER_H

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
 * @brief append a byte to the buffer.
 * @param[in] buffer buffer.
 * @param[in] byte byte to append.
 * @return false in case of error.
 */
bool LCH_BufferAppend(LCH_Buffer *buffer, char byte);

/**
 * @brief format- and print string to byte buffer.
 * @note buffer capacity is expanded if need be.
 * @param[in] buffer buffer.
 * @param[in] format format string.
 * @return false in case of failure.
 */
bool LCH_BufferPrintFormat(LCH_Buffer *buffer, const char *format, ...);

/**
 * @brief get length of buffer.
 * @param[in] buffer buffer.
 * @return length of buffer excluding the terminating null byte.
 */
size_t LCH_BufferLength(const LCH_Buffer *buffer);

/**
 * @brief destroy buffer.
 * @note noop if buffer is NULL.
 * @param[in] buffer buffer.
 */
void LCH_BufferDestroy(void *buffer);

/**
 * @brief get buffer.
 * @param [in] buffer buffer.
 * @return pointer to internal buffer.
 */
const char *LCH_BufferData(const LCH_Buffer *buffer);

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
 * @param bytes Byte buffer
 * @param hex Hexadecimal buffer
 * @return True on success, false on error
 */
bool LCH_BufferBytesToHex(LCH_Buffer *hex, const LCH_Buffer *bytes);

/**
 * @brief Converts hexadecimal string representation into bytes
 * @param bytes Byte buffer
 * @param hex Hexadecimal buffer
 * @return True on success, false on error
 */
bool LCH_BufferHexToBytes(LCH_Buffer *bytes, const LCH_Buffer *hex);

/**
 * @warning Makes the assumption that the input string is at least four bytes
 *          long (excluding the termination null-byte).
 */
bool LCH_BufferUnicodeToUTF8(LCH_Buffer *const buffer, const char *in);

void LCH_BufferChop(LCH_Buffer *const buffer, size_t offset);

/**
 * @brief Get pointer to internal buffer and destroy surrounding data structure.
 * @param buffer Buffer.
 * @return Pointer to internal char buffer.
 * @note Returned buffer must be freed with free(3). If you don't want to
 *       destroy the surrounding data structure, you can use LCH_BufferData()
 *       instead.
 */
char *LCH_BufferToString(LCH_Buffer *buffer);

/**
 * @brief Create a buffer containing string.
 * @param str String content of created buffer.
 * @return Buffer containing string.
 * @note String must be terminated by the NULL-byte.
 */
LCH_Buffer *LCH_BufferFromString(const char *str);

const LCH_Buffer *LCH_BufferStaticFromString(const char *str);

bool LCH_BufferReadFile(LCH_Buffer *buffer, const char *filename);

bool LCH_BufferWriteFile(const LCH_Buffer *buffer, const char *filename);

void LCH_BufferTrim(LCH_Buffer *buffer, char ch);

bool LCH_BufferAppendBuffer(LCH_Buffer *buffer, const LCH_Buffer *append);

bool LCH_BufferEqual(const LCH_Buffer *self, const LCH_Buffer *other);

int LCH_BufferCompare(const LCH_Buffer *self, const LCH_Buffer *other);

LCH_Buffer *LCH_BufferDuplicate(const LCH_Buffer *buffer);

#endif  // _LEECH_BUFFER_H
