/**
 * @file utils.c
 * Leech utility functions.
 */

#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct LCH_Buffer LCH_List; ///< List
typedef struct LCH_Buffer LCH_Dict; ///< Dict

/**
 * Create a list.
 * The list is allocated on the heap and must be freed with `LCH_ListDestroy`.
 * @param[out] list pointer to list.
 */
LCH_List *LCH_ListCreate();

/**
 * Create a dict.
 * The dict is allocated on the heap and must be freed with `LCH_DictDestroy`.
 * @param[out] dict pointer to dict.
 */
LCH_Dict *LCH_DictCreate();

/**
 * Get number of items in a list.
 * @param[in] self pointer to list.
 * @param[out] length length of list.
 */
size_t LCH_ListLength(const LCH_List *self);

/**
 * Get number of items in a dict.
 * @param[in] self pointer to dict.
 * @param[out] length length of dict.
 */
size_t LCH_DictLength(const LCH_Dict *self);

/**
 * Append item to a list.
 * @param[in] self
 * @param[in] value
 * @param[in] destroy
 * @param[out] success
 */
bool LCH_ListAppend(LCH_List *self, void *value, void (*destroy)(void *));

bool LCH_DictHasKey(const LCH_Dict *self, const char *key);

bool LCH_DictSet(LCH_Dict *self, const char *key, void *value,
                 void (*destroy)(void *));

void *LCH_ListGet(const LCH_List *self, size_t index);
void *LCH_DictGet(const LCH_Dict *self, const char *key);

void LCH_ListDestroy(LCH_List *self);
void LCH_DictDestroy(LCH_Dict *self);

LCH_List *LCH_SplitString(const char *str, const char *del);

#endif // _LEECH_UTILS_H
