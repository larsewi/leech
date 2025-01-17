#ifndef _LEECH_LEECH_H
#define _LEECH_LEECH_H

#include <stdbool.h>
#include <stdlib.h>

/****************************************************************************/
/*  Misc                                                                    */
/****************************************************************************/

/**
 * @brief Function signature used for comparison
 * @param left The left operand
 * @param right The right operand
 * @return -1 if left is less than right, 0 if left is equal to right, 1 if left
 *         is larger than one
 */
typedef int (*LCH_CompareFn)(const void *left, const void *right);

/**
 * @brief Function signature used for resource duplication
 * @param resource A pointer to the resource to be duplicated
 * @return A pointer to the duplicated resource or NULL in case of error
 */
typedef void *(*LCH_DuplicateFn)(const void *resource);

/****************************************************************************/
/*  Buffer                                                                  */
/****************************************************************************/

/**
 * @brief Self expanding byte buffer
 * @note Always null-byte terminated
 */
typedef struct LCH_Buffer LCH_Buffer;

/**
 * @brief Create a byte buffer
 * @return Byte buffer or NULL in case of failure
 */
LCH_Buffer *LCH_BufferCreate(void);

/**
 * @brief Append a byte to the buffer
 * @param[in] buffer The byte buffer
 * @param[in] byte The byte to append
 * @return False in case of failure
 */
bool LCH_BufferAppend(LCH_Buffer *buffer, char byte);

/**
 * @brief Format- and print string to byte buffer
 * @param buffer The byte buffer
 * @param format The format string
 * @return False in case of failure
 */
bool LCH_BufferPrintFormat(LCH_Buffer *buffer, const char *format, ...);

/**
 * @brief Get length of buffer
 * @param buffer The byte buffer
 * @return The length of buffer (excluding the terminating null byte)
 */
size_t LCH_BufferLength(const LCH_Buffer *buffer);

/**
 * @brief Get the internal buffer
 * @param buffer The byte buffer
 * @return Pointer to internal buffer
 */
const char *LCH_BufferData(const LCH_Buffer *buffer);

/**
 * @brief Truncate the buffer at given offset
 * @param buffer The byte buffer
 * @param offset The given offset
 */
void LCH_BufferChop(LCH_Buffer *const buffer, size_t offset);

/**
 * @brief Get the internal buffer and destroy surrounding data structure
 * @param buffer The byte buffer
 * @return Pointer to internal char buffer (never NULL)
 * @note The caller takes ownership of the internal buffer
 */
char *LCH_BufferToString(LCH_Buffer *buffer);

/**
 * @brief Create a byte buffer and fill it with the contents of a null-byte
 *        terminated string
 * @param str The null-byte terminated string
 * @return A byte buffer or NULL in case of error
 */
LCH_Buffer *LCH_BufferFromString(const char *str);

/**
 * @brief Destroy buffer
 * @param buffer Pointer to a byte buffer or NULL
 */
void LCH_BufferDestroy(void *buffer);

/**
 * @brief Compare two byte buffers for equality
 * @param left The left byte buffer operand
 * @param right The right byte buffer operand
 * @return True in case of equality
 */
bool LCH_BufferEqual(const LCH_Buffer *left, const LCH_Buffer *right);

/**
 * @brief Compare two byte buffers
 * @param left The left byte buffer operand
 * @param right The right byte buffer operand
 * @return -1 if left is less than right, 0 if left is equal to right, 1 if left
 *         is greater than right
 */
int LCH_BufferCompare(const LCH_Buffer *left, const LCH_Buffer *right);

/**
 * @brief Duplicate a byte buffer
 * @param buffer The byte buffer to duplicate
 * @return The duplicated byte buffer or NULL in case of failure
 */
LCH_Buffer *LCH_BufferDuplicate(const LCH_Buffer *buffer);

/**
 * @brief Read the contents of a file into a byte buffer
 * @param buffer The byte buffer
 * @param filename The filename
 * @return False in case of failure
 */
bool LCH_BufferReadFile(LCH_Buffer *buffer, const char *filename);

/**
 * @brief Write the contents of a byte buffer into a file
 * @param buffer The byte buffer
 * @param filename The filename
 * @return False in case of failure
 */
bool LCH_BufferWriteFile(const LCH_Buffer *buffer, const char *filename);

/****************************************************************************/
/*  List                                                                    */
/****************************************************************************/

/**
 * @brief Self expanding list
 */
typedef struct LCH_List LCH_List;

/**
 * @brief Create a list
 * @return The list or NULL in case of failure
 */
LCH_List *LCH_ListCreate(void);

/**
 * @brief Get the number of elements in the list
 * @param list The list
 * @return The number of elements
 */
size_t LCH_ListLength(const LCH_List *list);

/**
 * @brief Get the element at a given index from the list
 * @param list The list
 * @param index The index
 * @return Pointer to the element (caller does not take ownership of the
 *         element)
 * @warning Make sure not to use an index out of bounds
 */
void *LCH_ListGet(const LCH_List *list, size_t index);

/**
 * @brief Append value to the list
 * @param list The list
 * @param element The element
 * @param destroy The function to destroy the element or NULL
 * @return False in case of failure
 * @note The caller relieves the ownership of the element unless NULL is passed
 *       as the destroy function argument
 */
bool LCH_ListAppend(LCH_List *list, void *element, void (*destroy)(void *));

/**
 * @brief Destroy a list
 * @param list Pointer to the list
 */
void LCH_ListDestroy(void *list);

/**
 * @brief Assign an element to list at a given index
 * @param list The list
 * @param index The index
 * @param element The element
 * @note The caller relieves the ownership of the element unless NULL is passed
 *       as the destroy function argument
 */
void LCH_ListSet(LCH_List *list, size_t index, void *element,
                 void (*destroy)(void *));

/**
 * @brief Get the index of first occurrence of a given element in the list
 * @param list The list
 * @param element The element
 * @param compare The comparison function
 * @return The index of the first occurrence or the list size in case the
 *         element was not found
 */
size_t LCH_ListIndex(const LCH_List *list, const void *element,
                     LCH_CompareFn compare);

/**
 * @brief Sort a list in asending order
 * @param list The list
 * @param compare The comparison function
 */
void LCH_ListSort(LCH_List *const list, LCH_CompareFn compare);

/**
 * @brief Remove an element from the list at the given index
 * @param list The list
 * @param index The index
 * @return The removed element
 * @note The caller takes the ownership of the returned element and all elements
 *       to the right of the given index are shifted to the left
 * @warning Make sure not to use an index out of bounds
 */
void *LCH_ListRemove(LCH_List *list, size_t index);

/**
 * @brief Create a duplicate (deep copy) of a list
 * @param list The list
 * @param duplicate_fn The function used to duplicate the elements
 * @param destroy The function used to destroy the elements in the new copy
 * @return False in case of failure
 */
LCH_List *LCH_ListCopy(const LCH_List *list, LCH_DuplicateFn duplicate_fn,
                       void (*destroy)(void *));

/**
 * @brief Insert an element at a given index in a list
 * @param list The list
 * @param index The index
 * @param element The element
 * @param destroy The function used to destroy the element
 * @return False in case of error
 * @note The callee takes the ownership of the element and all elements
 *       from the given index are shifted to the right
 */
bool LCH_ListInsert(LCH_List *list, size_t index, void *element,
                    void (*destroy)(void *));

/****************************************************************************/
/*  Debug Messenger                                                         */
/****************************************************************************/

/**
 * @brief Bit used to enable debug log messages
 */
#define LCH_LOGGER_MESSAGE_TYPE_DEBUG_BIT (1 << 0)

/**
 * @brief Bit used to enable verbose log messages
 */
#define LCH_LOGGER_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)

/**
 * @brief Bit used to enable info log messages
 */
#define LCH_LOGGER_MESSAGE_TYPE_INFO_BIT (1 << 2)

/**
 * @brief Bit used to enable warning log messages
 */
#define LCH_LOGGER_MESSAGE_TYPE_WARNING_BIT (1 << 3)

/**
 * @brief Bit used to enable error log messages
 */
#define LCH_LOGGER_MESSAGE_TYPE_ERROR_BIT (1 << 4)

/**
 * @brief Function signature for logger callback function
 * @param severity The severity of the log message (contains the value of one of
 *                 the LCH_LOG_MESSAGE_TYPE_*_BIT macros)
 * @param message The log message
 */
typedef void (*LCH_LoggerCallbackFn)(unsigned char severity,
                                     const char *message);

/**
 * @brief Set logger severity
 * @param severity Zero or more of the LCH_LOG_MESSAGE_TYPE_*_BIT macros bitwise
 *        or'ed together
 */
void LCH_LoggerSeveritySet(unsigned char severity);

/**
 * @brief Override the default logger callback function
 * @param callback The function to be called on logging events
 */
void LCH_LoggerCallbackSet(LCH_LoggerCallbackFn callback);

/****************************************************************************/
/*  Main Interface                                                          */
/****************************************************************************/

/**
 * @brief Get the current version of leech
 * @return The current version
 */
const char *LCH_Version(void);

/**
 * @brief Record changes and append them to the block chain
 * @param work_dir The leech working directory
 * @return False in case of failure
 */
bool LCH_Commit(const char *work_dir);

/**
 * @brief Compute deltas containing the changes between the latest block and a
 *        given block
 * @param work_dir The leech working directory
 * @return A byte buffer containing the computed delta or NULL in case of
 *         failure
 */
LCH_Buffer *LCH_Diff(const char *work_dir, const char *block_id);

/**
 * @brief Compute deltas containing the changes between the current state and
 *        the genisis block
 * @param work_dir The leech working directory
 * @note This function should only be used as a recovery strategy in the case
 *       where pathing a proper delta fails
 * @return A byte buffer containing the computed delta or NULL in case of
 *         failure
 */
LCH_Buffer *LCH_Rebase(const char *work_dir);

/**
 * @brief Retrieve the history of given record in a given table between a given
 *        time interval
 * @param work_dir The leech working directory
 * @param table_id The table identifier
 * @param primary_fields The primary fields that identify the record
 * @return A byte buffer containing the history as a JSON data structure or NULL
 *         in case of failure
 */
LCH_Buffer *LCH_History(const char *work_dir, const char *table_id,
                        const LCH_List *primary_fields, double from, double to);

/**
 * @brief Patch outdated tables using computed deltas
 * @param work_dir The leech working directory
 * @param uid_field The name of the field containing unique identifiers
 * @param uid_value The unique identifier of the host that generated the delta
 * @param patch The delta
 * @param size The size of the delta
 * @return False in case of failure
 * @note If a delta computed from LCH_Diff fails, try patching again with a
 *       delta computed from LCH_Rebase. If this fails, you're F'ed
 */
bool LCH_Patch(const char *work_dir, const char *uid_field,
               const char *uid_value, const char *patch, size_t size);

/**
 * @brief Purge expired blocks in the block chain
 * @param work_dir The leech working directory
 * @note Use this function to prevent the blockchain from growing indefinitely
 */
bool LCH_Purge(const char *work_dir);

#endif  // _LEECH_LEECH_H
