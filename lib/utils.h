#ifndef _LEECH_UTILS
#define _LEECH_UTILS

#include "buffer.h"
#include "dict.h"
#include "leech.h"

/**
 * Split a string with delimitor.
 * @param[in] str string to split.
 * @param[in] del delimitor.
 * @param[out] list list of substrings.
 */
LCH_List *LCH_SplitString(const char *str, const char *del);

LCH_List *LCH_SplitStringSubstring(const char *str, const char *substr);

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
char *LCH_ReadFile(const char *path, size_t *length);

/**
 * @brief Write a text file.
 *
 * @param[in] path path to file.
 * @param[in] str string to write to file.
 * @return false in case of error.
 */
bool LCH_WriteFile(const char *path, const char *str);

LCH_Dict *LCH_TableToDict(const LCH_List *table, const char *primary,
                          const char *subsidiary);

LCH_List *LCH_DictToTable(const LCH_Dict *dict, const char *primary,
                          const char *subsidiary);

/**
 * @brief Marshal string to buffer.
 * @param[in] buffer.
 * @param[in] string to marshal.
 * @return false in case of error.
 * @note String can be retrieved with LCH_UnmarshalString.
 */
bool LCH_MarshalString(LCH_Buffer *buffer, const char *str);

/**
 * @breif Unmarshal string from buffer.
 * @param[in] buffer.
 * @param[out] unmarshaled string.
 * @return pointer to remaining buffer.
 * @note unmarshaled string must be freed with free(3).
 */
const char *LCH_UnmarshalString(const char *buffer, char **str);

#endif  // _LEECH_UTILS
