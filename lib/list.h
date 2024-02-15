#ifndef _LEECH_LIST_H
#define _LEECH_LIST_H

#include "leech.h"

typedef struct LCH_List LCH_List;
typedef int LCH_ListIndexCompareFn(const void *, const void *);

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

bool LCH_ListAppendStringDuplicate(LCH_List *list, const char *str);

/**
 * Destroy list and contents.
 * @param[in] list pointer to list.
 */
void LCH_ListDestroy(void *list);

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

void *LCH_ListRemove(LCH_List *list, size_t index);

#endif  // _LEECH_LIST_H
