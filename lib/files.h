#ifndef _LEECH_FILES_H
#define _LEECH_FILES_H

#include <stdbool.h>
#include <stdio.h>

#include "list.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */

/**
 * @brief Get the file size
 * @param file The file
 * @param size The variable in which to store the size
 * @return False in case of failure
 */
bool LCH_FileSize(FILE *file, size_t *size);

/**
 * @brief Check if file exists and is a regular file or directory
 * @param path The file path
 * @return True if file exists and is a regular file or directory
 * @note Does not follow symbolic links
 */
bool LCH_FileExists(const char *path);

/**
 * @brief Check if file exists and is a regular file
 * @param path The file path
 * @return True if file exists and is a regular file
 * @note Does not follow symbolic links
 */
bool LCH_FileIsRegular(const char *path);

/**
 * @brief Check if file exists and is a directory
 * @param path The file path
 * @return True if file exists and is a directory
 * @note Does not follow symbolic links
 */
bool LCH_FileIsDirectory(const char *path);

/**
 * @brief Join paths
 * @param path A buffer in which to write the joined path
 * @param path_max The maximum size of the path
 * @param n_items Number of path components to join
 * @param ... Path components
 * @return False in case of failure
 */
bool LCH_FilePathJoin(char *path, size_t path_max, size_t n_items, ...);

/**
 * @brief Recursively delete file or directory
 * @param path Path to the file or directory
 * @return False in case of failure
 * @note Does not follow symbolic links
 */
bool LCH_FileDelete(const char *path);

/**
 * @brief Create parent directories of a given file path
 * @param filename The file path
 * @return False in case of failure
 * @note The directories are created with the mode 0700
 */
bool LCH_FileCreateParentDirectories(const char *filename);

/**
 * @brief Return a list of files in a directory
 * @param path The directory path
 * @param filter_hidden Whether or not dot-files should be filtered
 * @return A list of files in directory or NULL in case of failure
 * @note The '.' and '..' files are always excluded
 */
LCH_List *LCH_FileListDirectory(const char *path, bool filter_hidden);

#endif  // _LEECH_FILES_H
