#ifndef _LEECH_DICT
#define _LEECH_DICT

#include <stdbool.h>
#include <stdlib.h>

#include "list.h"
#include "logger.h"

typedef struct LCH_Dict LCH_Dict;

/**
 * Create a dict.
 * The dict is allocated on the heap and must be freed with `LCH_DictDestroy`.
 * @return pointer to dict.
 */
LCH_Dict *LCH_DictCreate(void);

/**
 * Get number of items in a dict.
 * @param[in] dict pointer to dict.
 * @return length of dict.
 */
size_t LCH_DictLength(const LCH_Dict *dict);

/**
 * Check if dict has key.
 * @param[in] dict pointer to dict.
 * @param[in] key key to check.
 * @return true if key is present.
 */
bool LCH_DictHasKey(const LCH_Dict *dict, const char *key);

/**
 * @brief Get list of keys from dict.
 * @param dict pointer to dict.
 * @return list of keys.
 * @note List of keys must be freed with LCH_ListDestroy
 */
LCH_List *LCH_DictGetKeys(const LCH_Dict *dict);

/**
 * Set value if key is present or add key value pair.
 * @param[in] dict pointer to dict.
 * @param[in] key key to set.
 * @param[in] value data pointer.
 * @param[in] destroy data destroy function.
 * @return true if success.
 */
bool LCH_DictSet(LCH_Dict *dict, const char *key, void *value,
                 void (*destroy)(void *));

void *LCH_DictRemove(LCH_Dict *dict, const char *key);

/**
 * Get dict value.
 * @param[in] dict pointer to dict.
 * @param[in] key key assosiated with value.
 * @param[out] func function pointer.
 * @return data pointer
 */
void *LCH_DictGet(const LCH_Dict *dict, const char *key);

/**
 * Destroy dict and contents.
 * @param[in] dict pointer to dict.
 */
void LCH_DictDestroy(void *dict);

/**
 * Set minus based on key.
 * @param [in] left left operand.
 * @param [in] right right operand.
 * @param [in] duplicate function to duplicate value.
 * @param [in] destroy function to destroy value.
 * @return dict containing entries found in left operand that is not found in
 *         right operand.
 */
LCH_Dict *LCH_DictSetMinus(const LCH_Dict *left, const LCH_Dict *right,
                           void *(*duplicate)(const void *),
                           void (*destroy)(void *));

/**
 * Get set intersection where values are different.
 * @param [in] left left operand.
 * @param [in] right right operand.
 * @param [in] duplicate function to duplicate value.
 * @param [in] destroy function to destroy value.
 * @param [in] compare function to compare values.
 * @return dict containing entries found in both left and right operands, but
 *         where value is different.
 */
LCH_Dict *LCH_DictSetChangedIntersection(
    const LCH_Dict *left, const LCH_Dict *right,
    void *(*duplicate)(const void *), void (*destroy)(void *),
    int (*compare)(const void *, const void *));

#endif  // _LEECH_DICT
