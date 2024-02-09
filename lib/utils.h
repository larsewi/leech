#ifndef _LEECH_UTILS
#define _LEECH_UTILS

#include "buffer.h"
#include "dict.h"
#include "json.h"
#include "leech.h"

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
LCH_List *LCH_SplitString(const char *str, const char *del);

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

LCH_Dict *LCH_TableToDict(const LCH_List *table, const char *primary,
                          const char *subsidiary, bool has_header);

LCH_Json *LCH_TableToJsonObject(const LCH_List *table,
                                const LCH_List *primary_fields,
                                const LCH_List *subsidiary_fields);

LCH_List *LCH_DictToTable(const LCH_Dict *dict, const char *primary,
                          const char *subsidiary, bool keep_header);

/**
 * @brief Marshal string to buffer.
 * @param[in] buffer.
 * @param[in] string to marshal.
 * @return false in case of error.
 * @note String can be retrieved with LCH_UnmarshalString.
 */
bool LCH_MarshalString(LCH_Buffer *buffer, const char *str);

/**
 * @brief Unmarshal string from buffer.
 * @param[in] buffer.
 * @param[out] unmarshaled string.
 * @return pointer to remaining buffer.
 * @note unmarshaled string must be freed with free(3).
 */
const char *LCH_UnmarshalString(const char *buffer, char **const str);

const char *LCH_UnmarshalBinary(const char *buffer, char **str);

bool LCH_MessageDigest(const unsigned char *message, size_t length,
                       LCH_Buffer *digest);

bool LCH_ParseNumber(const char *str, long *number);

char *LCH_VersionToString(size_t major, size_t minor, size_t patch);

bool LCH_ParseVersion(const char *str, size_t *major, size_t *minor,
                      size_t *patch);

char *LCH_StringDuplicate(const char *str);

char *LCH_StringFormat(const char *format, ...);

/**
 * @brief Destroy NULL-terminated string array.
 * @param array One dimentional array of strings.
 */
void LCH_StringArrayDestroy(void *array);

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
char **LCH_StringListToStringArray(const LCH_List *list);

/**
 * @brief Create a NULL-terminated string table from list.
 * @param list Two dimentional list of strings.
 * @return Two dimentional array of strings.
 */
char ***LCH_StringListTableToStringArrayTable(const LCH_List *list);

/**
 * @brief Create a list from NULL-terminated string array.
 * @param str_array One dimentional array of strings.
 * @return One dimentional list of strings.
 */
LCH_List *LCH_StringArrayToStringList(char **str_array);

/**
 * @brief Create a list from NULL-terminated string table.
 * @param str_array Two dimentional array of strings.
 * @return Two dimentional list of strings.
 */
LCH_List *LCH_StringArrayTableToStringListTable(char ***str_table);

bool LCH_CreateParentDirectories(const char *filename);

#endif  // _LEECH_UTILS
