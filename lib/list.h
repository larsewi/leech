#ifndef _LEECH_LIST_H
#define _LEECH_LIST_H

#include "leech.h"

/**
 * Assing value to list item at index.
 * @param[in] self pointer to list.
 * @param[in] index index of item.
 * @param[in] value value to assign.
 */
void LCH_ListSet(LCH_List *self, size_t index, void *value,
                 void (*destroy)(void *));

/**
 * Get index of first occurance of value in list.
 * @param[in] self pointer to list.
 * @param[in] value value to find.
 * @param[in] compare comparison function.
 */
size_t LCH_ListIndex(const LCH_List *self, const void *value,
                     int (*compare)(const void *, const void *));

/**
 * Sort list.
 * @param[in] self pointer to list.
 * @param[in] compare comparison function.
 */
void LCH_ListSort(LCH_List *const self,
                  int (*compare)(const void *, const void *));

/**
 * Destroy list except contents.
 * @param[in] self pointer to list.
 */
void LCH_ListDestroyShallow(LCH_List *self);

LCH_List *LCH_ListMoveElements(LCH_List *destination, LCH_List *source);

#endif  // _LEECH_LIST_H
