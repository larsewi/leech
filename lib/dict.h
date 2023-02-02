#ifndef _LEECH_DICT
#define _LEECH_DICT

#include <stdbool.h>
#include <stdlib.h>

#include "leech.h"

typedef struct LCH_DictIter LCH_DictIter;

/**
 * Set minus based on key.
 * @param [in] self left operand.
 * @param [in] other right operand.
 * @param [in] duplicate function to duplicate value.
 * @param [in] destroy function to destroy value.
 * @return dict containing entries found in self that is not found in other.
 */
LCH_Dict *LCH_DictSetMinus(const LCH_Dict *self, const LCH_Dict *other,
                           void *(*duplicate)(const void *),
                           void (*destroy)(void *));

/**
 * Get set intersection where values are different.
 * @param [in] self left operand.
 * @param [in] other right operand.
 * @param [in] duplicate function to duplicate value.
 * @param [in] destroy function to destroy value.
 * @param [in] compare function to compare values.
 * @return dict containing entries found in both self and other, but where
 *         value is different.
 */
LCH_Dict *LCH_DictSetChangedIntersection(
    const LCH_Dict *self, const LCH_Dict *other,
    void *(*duplicate)(const void *), void (*destroy)(void *),
    int (*compare)(const void *, const void *));

/**
 * Create a dict iterator.
 * @param [in] dict dict to create iterator from.
 * @return pointer to iterator.
 * @note iterator must be freed with free.
 */
LCH_DictIter *LCH_DictIterCreate(const LCH_Dict *const dict);

/**
 * Find next element in iterator.
 * @param [in] iter pointer to iterator.
 * @return true if new element was found.
 */
bool LCH_DictIterHasNext(LCH_DictIter *iter);

/**
 * Get key from found element.
 * @param [in] pointer to iterator.
 * @return key from element.
 */
char *LCH_DictIterGetKey(const LCH_DictIter *iter);

/**
 * Get value from found element.
 * @param [in] pointer to iterator.
 * @return value from element.
 */
void *LCH_DictIterGetValue(const LCH_DictIter *iter);

#endif  // _LEECH_DICT
