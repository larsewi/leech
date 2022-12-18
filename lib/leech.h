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
 * Destroy list and contents.
 * @param[in] self pointer to list.
 */
void LCH_ListDestroy(LCH_List *self);

/**
 * Destroy list except contents.
 * @param[in] self pointer to list.
 */
void LCH_ListDestroyShallow(LCH_List *self);

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
/*  Buffer                                                                  */
/****************************************************************************/

typedef struct LCH_Buffer LCH_Buffer;

LCH_Buffer *LCH_BufferCreate(void);

bool LCH_BufferAppend(LCH_Buffer *self, const char *format, ...);

size_t LCH_BufferLength(LCH_Buffer *self);

char *LCH_BufferGet(LCH_Buffer *self);

void LCH_BufferDestroy(LCH_Buffer *self);

/****************************************************************************/
/*  CSV                                                                     */
/****************************************************************************/

LCH_List *LCH_ParseCSV(const char *str);

LCH_Buffer *LCH_ComposeCSV(const LCH_List *table);

/****************************************************************************/
/*  Utils                                                                   */
/****************************************************************************/

/**
 * Split a string with delimitor.
 * @param[in] str string to split.
 * @param[in] del delimitor.
 * @param[out] list list of substrings.
 */
LCH_List *LCH_SplitString(const char *str, const char *del);

bool LCH_StringStartsWith(const char *str, const char *substr);

char *LCH_StringStrip(char *str, const char *charset);

bool LCH_FileSize(FILE *file, size_t *size);

/****************************************************************************/
/*  Table                                                                   */
/****************************************************************************/

typedef struct LCH_Table LCH_Table;

typedef struct LCH_TableCreateInfo {
  const char *primaryFields;
  const char *subsidiaryFields;
  const void *readLocator;
  const void *writeLocator;
  LCH_List *(*readCallback)(const void *);
  bool (*writeCallback)(const void *, const LCH_List *);
} LCH_TableCreateInfo;

LCH_Table *LCH_TableCreate(const LCH_TableCreateInfo *createInfo);

void LCH_TableDestroy(LCH_Table *table);

/****************************************************************************/
/*  Instance                                                                */
/****************************************************************************/

typedef struct LCH_Instance LCH_Instance;
typedef struct LCH_InstanceCreateInfo {
  const char *instanceID;
  const char *workDir;
} LCH_InstanceCreateInfo;

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo);
void LCH_InstanceDestroy(LCH_Instance *instance);

#endif  // _LEECH_LEECH_H
