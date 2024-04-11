#ifndef _LEECH_UTILS
#define _LEECH_UTILS

#include <stdio.h>

#include "buffer.h"
#include "dict.h"
#include "json.h"
#include "leech.h"
#include "list.h"

void *LCH_Allocate(size_t size);

/**
 * Check if two strings are equivalent.
 * @param first first string to check.
 * @param second second string to check.
 * @return true if strings are equivalent, else false.
 */
bool LCH_StringEqual(const char *str1, const char *str2);

/**
 * @brief Split string into a list based on delimiters.
 * @param[in] str String to split.
 * @param[in] del Delimiters to split on.
 * @return List of substrings split on delimiter.
 */
LCH_List *LCH_StringSplit(const char *str, const char *del);

char *LCH_StringJoin(const LCH_List *list, const char *del);

bool LCH_StringStartsWith(const char *str, const char *substr);

char *LCH_StringStrip(char *str, const char *charset);

bool LCH_FileSize(FILE *file, size_t *size);

bool LCH_FileExists(const char *path);

bool LCH_IsRegularFile(const char *path);

bool LCH_IsDirectory(const char *path);

bool LCH_PathJoin(char *path, size_t path_max, size_t n_items, ...);

/**
 * @brief Read a text file.
 *
 * @note Returned string must be freed with free(3).
 * @param[in] path path to file.
 * @param[out] length If not NULL, it is set to be the string length, excluding
 *                    the terminating null byte.
 * @return string containing the file content, or NULL in case of error.
 */
char *LCH_FileRead(const char *path, size_t *length);

/**
 * @brief Write a text file.
 *
 * @param[in] path path to file.
 * @param[in] str string to write to file.
 * @return false in case of error.
 */
bool LCH_FileWrite(const char *path, const char *str);

LCH_Json *LCH_TableToJsonObject(const LCH_List *table,
                                const LCH_List *primary_fields,
                                const LCH_List *subsidiary_fields);

bool LCH_MessageDigest(const unsigned char *message, size_t length,
                       LCH_Buffer *digest);

bool LCH_ParseNumber(const char *str, long *number);

bool LCH_ParseVersion(const char *str, size_t *major, size_t *minor,
                      size_t *patch);

char *LCH_StringDuplicate(const char *str);

char *LCH_StringFormat(const char *format, ...);

/**
 * @brief Destroy NULL-terminated string array.
 * @param array One dimentional array of strings.
 */
void LCH_StringArrayDestroy(void *array);

size_t LCH_StringArrayLength(const char *const *str_array);

char **LCH_StringArrayInsert(const char *const *str_array, size_t position);

/**
 * @brief Destroy NULL-terminated string table.
 * @param table Two dimentional array of strings.
 */
void LCH_StringArrayTableDestroy(void *table);

/**
 * @brief Create a NULL-terminated string array from list.
 * @param list One dimentional list of strings.
 * @return One dimentional array of strings.
 */
char **LCH_RecordToStringArray(const LCH_List *list);

/**
 * @brief Create a NULL-terminated string table from list.
 * @param list Two dimentional list of strings.
 * @return Two dimentional array of strings.
 */
char ***LCH_TableToStringArrayTable(const LCH_List *list);

/**
 * @brief Create a list from NULL-terminated string array.
 * @param str_array One dimentional array of strings.
 * @return One dimentional list of strings.
 */
LCH_List *LCH_StringArrayToRecord(const char *const *str_array);

/**
 * @brief Create a list from NULL-terminated string table.
 * @param str_array Two dimentional array of strings.
 * @return Two dimentional list of strings.
 */
LCH_List *LCH_StringArrayTableToTable(const char *const *const *str_table);

bool LCH_CreateParentDirectories(const char *filename);

bool LCH_ListInsertBufferDuplicate(LCH_List *list, size_t index,
                                   const LCH_Buffer *buffer);

bool LCH_ListAppendBufferDuplicate(LCH_List *list, const LCH_Buffer *buffer);

char *LCH_StringTruncate(const char *str, size_t len, size_t max);

#endif  // _LEECH_UTILS
