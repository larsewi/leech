#ifndef _LEECH_DICT
#define _LEECH_DICT

#include <stdbool.h>
#include <stdlib.h>

#include "leech.h"

typedef struct LCH_DictIter LCH_DictIter;

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
