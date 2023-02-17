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
 * @param[in] self pointer to list.
 * @return length of list.
 */
size_t LCH_ListLength(const LCH_List *self);

/**
 * Get list item.
 * @param[in] self pointer to list.
 * @param[in] index index of item.
 * @return data pointer.
 */
void *LCH_ListGet(const LCH_List *self, size_t index);

/**
 * Append value to a list.
 * @param[in] self pointer to dict.
 * @param[in] value data pointer.
 * @param[in] destroy data destroy function.
 * @return true if success.
 */
bool LCH_ListAppend(LCH_List *self, void *value, void (*destroy)(void *));

/**
 * Destroy list and contents.
 * @param[in] self pointer to list.
 */
void LCH_ListDestroy(LCH_List *self);

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
 * @param[in] self pointer to dict.
 * @return length of dict.
 */
size_t LCH_DictLength(const LCH_Dict *self);

/**
 * Check if dict has key.
 * @param[in] self pointer to dict.
 * @param[in] key key to check.
 * @return true if key is present.
 */
bool LCH_DictHasKey(const LCH_Dict *self, const char *key);

/**
 * @brief Get list of keys from dict.
 * @param dict pointer to dict.
 * @return list of keys.
 * @note List of keys must be freed with LCH_ListDestroy
 */
LCH_List *LCH_DictGetKeys(const LCH_Dict *self);

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

void *LCH_DictRemove(LCH_Dict *self, const char *key);

/**
 * Get dict value.
 * @param[in] self pointer to dict.
 * @param[in] key key assosiated with value.
 * @param[out] func function pointer.
 * @return data pointer
 */
void *LCH_DictGet(const LCH_Dict *self, const char *key);

/**
 * Destroy dict and contents.
 * @param[in] self pointer to dict.
 */
void LCH_DictDestroy(LCH_Dict *self);

/****************************************************************************/
/*  Table                                                                   */
/****************************************************************************/

typedef struct LCH_Table LCH_Table;

typedef struct LCH_TableCreateInfo {
  const char *identifier;
  const char *primary_fields;
  const char *subsidiary_fields;
  const void *read_locator;
  const void *write_locator;
  LCH_List *(*read_callback)(const void *);
  bool (*write_callback)(const void *, const LCH_List *);
  bool (*insert_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*delete_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
  bool (*update_callback)(const void *, const char *, const char *,
                          const LCH_Dict *);
} LCH_TableCreateInfo;

LCH_Table *LCH_TableCreate(const LCH_TableCreateInfo *createInfo);

void LCH_TableDestroy(LCH_Table *table);

/****************************************************************************/
/*  Instance                                                                */
/****************************************************************************/

typedef struct LCH_Instance LCH_Instance;

typedef struct LCH_InstanceCreateInfo {
  const char *work_dir;
} LCH_InstanceCreateInfo;

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo);

bool LCH_InstanceAddTable(LCH_Instance *instance, LCH_Table *table);

bool LCH_InstanceCommit(const LCH_Instance *instance);

char *LCH_InstanceDelta(const LCH_Instance *instance, const char *block_id,
                        size_t *buf_len);

bool LCH_InstancePatch(const LCH_Instance *self, const char *patch,
                       const char *uid_field, const char *uid_value,
                       size_t size);

void LCH_InstanceDestroy(LCH_Instance *instance);

#endif  // _LEECH_LEECH_H
