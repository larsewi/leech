#ifndef _LEECH_LEECH_H
#define _LEECH_LEECH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/****************************************************************************/
/*  Debug Messenger                                                         */
/****************************************************************************/

#define LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT (1 << 0)
#define LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT (1 << 1)
#define LCH_DEBUG_MESSAGE_TYPE_INFO_BIT (1 << 2)
#define LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT (1 << 3)
#define LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT (1 << 4)

#define LCH_LOG_DEBUG(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT, __VA_ARGS__)
#define LCH_LOG_VERBOSE(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT, __VA_ARGS__)
#define LCH_LOG_INFO(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LCH_LOG_WARNING(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT, __VA_ARGS__)
#define LCH_LOG_ERROR(...) \
  LCH_LogMessage(LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT, __VA_ARGS__)

typedef struct LCH_DebugMessengerInitInfo {
  unsigned char severity;
  void (*messageCallback)(unsigned char, const char *);
} LCH_DebugMessengerInitInfo;

void LCH_DebugMessengerInit(const LCH_DebugMessengerInitInfo *const initInfo);

void LCH_LogMessage(unsigned char severity, const char *format, ...);

void LCH_DebugMessengerCallbackDefault(unsigned char severity,
                                       const char *message);

/****************************************************************************/
/*  List                                                                    */
/****************************************************************************/

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

/**
 * Destroy list and contents.
 * @param[in] list pointer to list.
 */
void LCH_ListDestroy(void *list);

/****************************************************************************/
/*  Dict                                                                    */
/****************************************************************************/

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

bool LCH_Commit(const char *work_dir);

char *LCH_Diff(const char *work_dir, const char *block_id, size_t *buf_len);

bool LCH_Patch(const char *work_dir, const char *patch, const char *uid_field,
               const char *uid_value, size_t size);

#endif  // _LEECH_LEECH_H
