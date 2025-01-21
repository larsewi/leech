#ifndef _LEECH_HEAD_H
#define _LEECH_HEAD_H

#include <stdbool.h>

/**
 * @brief Get the reference stored in the HEAD file
 * @param name The name of the HEAD file
 * @param work_dir The leech working directory
 * @return The reference (block identifier) of the head of the chain or NULL in
 *         case of failure
 * @note The HEAD file should always contain a reference to the head of the
 *       chain
 */
char *LCH_HeadGet(const char *name, const char *work_dir);

/**
 * @brief Set the reference stored in the HEAD file
 * @param name The name of the HEAD file
 * @param work_dir The leech working directory
 * @param block_id The reference (block identifier)
 * @return False in case of failure
 * @note The HEAD file should always contain a reference to the head of the
 *       chain
 */
bool LCH_HeadSet(const char *name, const char *work_dir, const char *block_id);

#endif  // _LEECH_HEAD_H
