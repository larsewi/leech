#ifndef _LEECH_LIST_H
#define _LEECH_LIST_H

#include <stdbool.h>
#include <stdlib.h>

#include "definitions.h"

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

bool LCH_ListAppendStringDuplicate(LCH_List *list, const char *str);

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

#endif  // _LEECH_LIST_H
