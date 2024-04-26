#ifndef _LEECH_FILES_H
#define _LEECH_FILES_H

#include <stdbool.h>
#include <stdio.h>

#include "list.h"

bool LCH_FileSize(FILE *file, size_t *size);

bool LCH_FileExists(const char *path);

bool LCH_FileIsRegular(const char *path);

bool LCH_FileIsDirectory(const char *path);

bool LCH_FilePathJoin(char *path, size_t path_max, size_t n_items, ...);

/**
 * @brief Delete file.
 * @param filename File to delte.
 * @return True on success, otherwise false.
 */
bool LCH_FileDelete(const char *filename);

bool LCH_FileCreateParentDirectories(const char *filename);

LCH_List *LCH_FileListDirectory(const char *path, bool filter_hidden);

#endif  // _LEECH_FILES_H
