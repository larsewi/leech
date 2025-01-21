#ifndef _LEECH_DICT
#define _LEECH_DICT

#include <stdbool.h>
#include <stdlib.h>

#include "buffer.h"
#include "list.h"
#include "logger.h"

/**
 * @brief Dictionary (hash map) containing key-value pairs
 */
typedef struct LCH_Dict LCH_Dict;

/**
 * @brief Create a dictionary
 * @return The dictionary or NULL in case of failure
 */
LCH_Dict *LCH_DictCreate(void);

/**
 * @brief Get the number of key-value pairs the dictionary
 * @param dict The dictionary
 * @return The number of key-value pairs
 */
size_t LCH_DictLength(const LCH_Dict *dict);

/**
 * @brief Check for existence of an entry with the given key in dictionary
 * @param dict The dictionary
 * @param key The key
 * @return False in case of failure
 */
bool LCH_DictHasKey(const LCH_Dict *dict, const LCH_Buffer *key);

/**
 * @brief Get a list of the existing keys in the dictionary
 * @param dict The dictionary
 * @return List of existing keys or NULL in case of failure
 */
LCH_List *LCH_DictGetKeys(const LCH_Dict *dict);

/**
 * @brief Add or update a key-value pair in the dictionary
 * @param dict The dictionary
 * @param key The key
 * @param value The value
 * @param destroy The function to destroy the new value or NULL
 * @return False in case of failure
 * @warning If the key already exists, the previous value is destroyed unless
 *          the destroy argument was NULL when it was added
 */
bool LCH_DictSet(LCH_Dict *dict, const LCH_Buffer *key, void *value,
                 void (*destroy)(void *));

/**
 * @brief Remove a key-value pair from the dictionary
 * @param dict The dictionary
 * @param key The key
 * @return The value
 * @note The caller takes ownership of the returned value
 */
void *LCH_DictRemove(LCH_Dict *dict, const LCH_Buffer *key);

/**
 * @brief Get the value of a key-value pair in the dictionary given the key
 * @param dict The Dictionary
 * @param key The key
 * @return The value
 * @note The caller takes ownership of the returned value
 * @warning Make sure the key exists before calling this function
 */
const void *LCH_DictGet(const LCH_Dict *dict, const LCH_Buffer *key);

/**
 * @brief Destroy dictionary and all it's key-value pairs
 * @param dict The dictionary
 * @note The value of each entry is free'd using their appointed destroy
 *       function unless destroy function was set to NULL
 */
void LCH_DictDestroy(void *dict);

#endif  // _LEECH_DICT
