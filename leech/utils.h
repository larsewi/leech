/**
 * @file utils.c
 * Leech utility functions.
 */

#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct LCH_Buffer LCH_List;
typedef struct LCH_Buffer LCH_Dict;

/**
 * Create a list.
 * The list is allocated on the heap and must be freed with `LCH_ListDestroy`.
 * @return pointer to list.
 */
LCH_List *LCH_ListCreate(void);

/**
 * Create a dict.
 * The dict is allocated on the heap and must be freed with `LCH_DictDestroy`.
 * @return pointer to dict.
 */
LCH_Dict *LCH_DictCreate(void);

/**
 * Get number of items in a list.
 * @param[in] self pointer to list.
 * @return length of list.
 */
size_t LCH_ListLength(const LCH_List *self);

/**
 * Get number of items in a dict.
 * @param[in] self pointer to dict.
 * @return length of dict.
 */
size_t LCH_DictLength(const LCH_Dict *self);

/**
 * Append value to a list.
 * @param[in] self pointer to dict.
 * @param[in] value data pointer.
 * @param[in] destroy data destroy function.
 * @return true if success.
 */
bool LCH_ListAppend(LCH_List *self, void *value, void (*destroy)(void *));

/**
 * Check if dict has key.
 * @param[in] self pointer to dict.
 * @param[in] key key to check.
 * @return true if key is present.
 */
bool LCH_DictHasKey(const LCH_Dict *self, const char *key);

/**
 * Set value if key is present or add key value pair.
 * @param[in] self pointer to dict.
 * @param[in] key key to set.
 * @param[in] value data pointer.
 * @param[in] destroy data destroy function.
 * @return true if success.
 */
bool LCH_DictSet(LCH_Dict *self, const char *key, void *value,
                 void (*destroy)(void *));

/**
 * Get list item.
 * @param[in] self pointer to list.
 * @param[in] index index of item.
 * @return data pointer.
 */
void *LCH_ListGet(const LCH_List *self, size_t index);

/**
 * Get dict value.
 * @param[in] self pointer to dict.
 * @param[in] key key assosiated with value.
 * @param[out] func function pointer.
 * @return data pointer
 */
void *LCH_DictGet(const LCH_Dict *self, const char *key);

/**
 * Destroy list and contents.
 * @param[in] self pointer to list.
 */
void LCH_ListDestroy(LCH_List *self);

/**
 * Destroy dict and contents.
 * @param[in] self pointer to dict.
 */
void LCH_DictDestroy(LCH_Dict *self);

/**
 * Split a string with delimitor.
 * @param[in] str string to split.
 * @param[in] del delimitor.
 * @param[out] list list of substrings.
 */
LCH_List *LCH_SplitString(const char *str, const char *del);

bool LCH_FileWriteCSVField(FILE *file, const char *field);

bool LCH_FileWriteCSVRecord(FILE *const file, const LCH_List *const record);

bool LCH_FileWriteCSVTable(FILE *const file, const LCH_List *const table);

#endif // _LEECH_UTILS_H
