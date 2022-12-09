#ifndef _LEECH_LIST_H
#define _LEECH_LIST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct LCH_List LCH_List;

/**
 * Create a list.
 * The list is allocated on the heap and must be freed with `LCH_ListDestroy`.
 * @return pointer to list.
 */
LCH_List *LCH_ListCreate(void);

/**
 * Get number of items in a list.
 * @param[in] self pointer to list.
 * @return length of list.
 */
size_t LCH_ListLength(const LCH_List *self);

/**
 * Append value to a list.
 * @param[in] self pointer to dict.
 * @param[in] value data pointer.
 * @param[in] destroy data destroy function.
 * @return true if success.
 */
bool LCH_ListAppend(LCH_List *self, void *value, void (*destroy)(void *));

/**
 * Get list item.
 * @param[in] self pointer to list.
 * @param[in] index index of item.
 * @return data pointer.
 */
void *LCH_ListGet(const LCH_List *self, size_t index);

/**
 * Destroy list and contents.
 * @param[in] self pointer to list.
 */
void LCH_ListDestroy(LCH_List *self);

#endif  // _LEECH_LIST_H
