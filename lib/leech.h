#ifndef _LEECH_LEECH_H
#define _LEECH_LEECH_H

#include <stdbool.h>
#include <stdlib.h>

/****************************************************************************/
/*  Misc                                                                    */
/****************************************************************************/

typedef int (*LCH_CompareFn)(const void *, const void *);
typedef void *(*LCH_DuplicateFn)(const void *);

/****************************************************************************/
/*  Buffer                                                                  */
/****************************************************************************/

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
 * @brief get buffer.
 * @param [in] buffer buffer.
 * @return pointer to internal buffer.
 */
const char *LCH_BufferData(const LCH_Buffer *buffer);

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

/**
 * @brief destroy buffer.
 * @note noop if buffer is NULL.
 * @param[in] buffer buffer.
 */
void LCH_BufferDestroy(void *buffer);

bool LCH_BufferEqual(const LCH_Buffer *self, const LCH_Buffer *other);

int LCH_BufferCompare(const LCH_Buffer *self, const LCH_Buffer *other);

LCH_Buffer *LCH_BufferDuplicate(const LCH_Buffer *buffer);

bool LCH_BufferReadFile(LCH_Buffer *buffer, const char *filename);

bool LCH_BufferWriteFile(const LCH_Buffer *buffer, const char *filename);

/****************************************************************************/
/*  List                                                                    */
/****************************************************************************/

typedef struct LCH_List LCH_List;

/**
 * Create a list.
 * The list is allocated on the heap and must be freed with `LCH_ListDestroy`.
 * @return pointer to list.
 */
LCH_List *LCH_ListCreate(void);

/**
 * Get number of items in a list.
 * @param[in] list pointer to list.
 * @return length of list.
 */
size_t LCH_ListLength(const LCH_List *list);

/**
 * Get list item.
 * @param[in] list pointer to list.
 * @param[in] index index of item.
 * @return data pointer.
 */
void *LCH_ListGet(const LCH_List *list, size_t index);

/**
 * Append value to a list.
 * @param[in] list pointer to dict.
 * @param[in] value data pointer.
 * @param[in] destroy data destroy function.
 * @return true if success.
 */
bool LCH_ListAppend(LCH_List *list, void *value, void (*destroy)(void *));

/**
 * Destroy list and contents.
 * @param[in] list pointer to list.
 */
void LCH_ListDestroy(void *list);

/**
 * Assing value to list item at index.
 * @param[in] list pointer to list.
 * @param[in] index index of item.
 * @param[in] value value to assign.
 */
void LCH_ListSet(LCH_List *list, size_t index, void *value,
                 void (*destroy)(void *));

/**
 * Get index of first occurance of value in list.
 * @param[in] list pointer to list.
 * @param[in] value value to find.
 * @param[in] compare comparison function.
 */
size_t LCH_ListIndex(const LCH_List *list, const void *value,
                     LCH_CompareFn compare);

/**
 * Sort list.
 * @param[in] list pointer to list.
 * @param[in] compare comparison function.
 */
void LCH_ListSort(LCH_List *const list, LCH_CompareFn compare);

void *LCH_ListRemove(LCH_List *list, size_t index);

LCH_List *LCH_ListCopy(const LCH_List *list, LCH_DuplicateFn duplicate_fn,
                       void (*destroy)(void *));

bool LCH_ListInsert(LCH_List *list, size_t index, void *value,
                    void (*destroy)(void *));

/****************************************************************************/
/*  Debug Messenger                                                         */
/****************************************************************************/

#define LCH_LOGGER_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_LOGGER_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_LOGGER_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_LOGGER_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_LOGGER_MESSAGE_TYPE_ERROR_BIT (1 << 4)

typedef void (*LCH_LoggerCallbackFn)(unsigned char, const char *);

void LCH_LoggerSeveritySet(unsigned char severity);

void LCH_LoggerCallbackSet(LCH_LoggerCallbackFn callback);

/****************************************************************************/
/*  Main Interface                                                          */
/****************************************************************************/

const char *LCH_Version(void);

bool LCH_Commit(const char *work_dir);

LCH_Buffer *LCH_Diff(const char *work_dir, const char *block_id);

LCH_Buffer *LCH_Rebase(const char *work_dir);

LCH_Buffer *LCH_History(const char *work_dir, const char *table_id,
                        const LCH_List *primary_fields, double from, double to);

bool LCH_Patch(const char *work_dir, const char *uid_field,
               const char *uid_value, const char *patch, size_t size);

#endif  // _LEECH_LEECH_H
