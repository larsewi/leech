#ifndef _LEECH_LIST_H
#define _LEECH_LIST_H

#include "leech.h"

LCH_List *LCH_ListCreateWithCapacity(size_t capacity);

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
                     int (*compare)(const void *, const void *));

/**
 * Sort list.
 * @param[in] list pointer to list.
 * @param[in] compare comparison function.
 */
void LCH_ListSort(LCH_List *const list,
                  int (*compare)(const void *, const void *));

LCH_List *LCH_ListMoveElements(LCH_List *destination, LCH_List *source);

void LCH_ListSwap(LCH_List *list, size_t i, size_t j);

#endif  // _LEECH_LIST_H
