#ifndef _LEECH_DICT
#define _LEECH_DICT

#include <stdbool.h>
#include <stdlib.h>

#include "buffer.h"
#include "definitions.h"
#include "list.h"
#include "logger.h"

typedef struct LCH_Dict LCH_Dict;

/**
 * @brief Create a dictionary.
 * @return Pointer to dictionary or NULL in case of memory errors.
 * @note Returned dictionary must be freed with LCH_DictDestroy().
 */
LCH_Dict *LCH_DictCreate(void);

/**
 * @brief Get number of entries in dictionary.
 * @param dict Pointer to dictionary.
 * @return Number of dictionary entries.
 */
size_t LCH_DictLength(const LCH_Dict *dict);

/**
 * @brief Check for existance of entry with key in dictionary.
 * @param dict Pointer to dictionary.
 * @param key Key to check.
 * @return True if entry with key exists.
 */
bool LCH_DictHasKey(const LCH_Dict *dict, const LCH_Buffer *key);

/**
 * @brief Get list of existing keys in dictionary.
 * @param dict Pointer to dictionary.
 * @return List of existing keys or NULL in case of memory errors.
 * @note List must be freed with LCH_ListDestroy().
 */
LCH_List *LCH_DictGetKeys(const LCH_Dict *dict);

/**
 * @brief Create/update entry in dictionary.
 * @param dict Pointer to dictionary.
 * @param key Key of entry.
 * @param value Value of entry.
 * @param destroy Function called to free value upon destruction if not NULL.
 * @note This function takes ownership of passed value.
 * @return True on success or false in case of memory errors.
 */
bool LCH_DictSet(const LCH_Dict *dict, const LCH_Buffer *key, void *value,
                 void (*destroy)(void *));

/**
 * @brief Remove entry from dictionary.
 * @param dict Pointer to dictionary.
 * @param key Key of entry.
 * @return Value of entry.
 * @note This function relieves ownership of returned value and cannot fail.
 */
void *LCH_DictRemove(const LCH_Dict *dict, const LCH_Buffer *key);

/**
 * @brief Get value of entry with key in dictionary.
 * @param dict Pointer to dictionary.
 * @param key Key of entry.
 * @return Value of entry.
 * @note This function does not relieve ownership of returned value and cannot
 *       fail.
 */
const void *LCH_DictGet(const LCH_Dict *dict, const LCH_Buffer *key);

/**
 * @brief Destroy dictionary and all entries.
 * @param dict Pointer to dictionary.
 * @note The value of each entry is free'd using their appointed destroy
 *       function unless destroy function was set to NULL.
 */
void LCH_DictDestroy(void *dict);

/**
 * @brief Returns a dictionary containing the result of a set minus operation
 *        based on keys of each entry.
 * @param left Left set minus operand.
 * @param right Right set minus operand.
 * @param duplicate Function to duplicate the value.
 * @param destroy Function to destroy the value.
 * @return Dictionary containing the results of the set minus operation or NULL
 *         in case of memory errors.
 * @note Returned dictionary must be freed with a call to LCH_DictDestroy().
 */
LCH_Dict *LCH_DictSetMinus(const LCH_Dict *left, const LCH_Dict *right,
                           void *(*duplicate)(const void *),
                           void (*destroy)(void *));

/**
 * @brief Returns a dictionary containing the result of a set intersect
 *        operation based on keys, but where values differ.
 * @param left Left set intersect operand.
 * @param right Right set intersect operand.
 * @param duplicate Function to duplicate the value.
 * @param destroy Function to destroy the value.
 * @param compare Function to compare two values.
 * @return Dictionary containing the results of the set intersect operation or
 *         NULL in case of memory errors.
 * @note Returned dictionary must be freed with a call to LCH_DictDestroy().
 */
LCH_Dict *LCH_DictSetChangedIntersection(const LCH_Dict *left,
                                         const LCH_Dict *right,
                                         LCH_DuplicateFn duplicate,
                                         void (*destroy)(void *),
                                         LCH_CompareFn compare);

#endif  // _LEECH_DICT
