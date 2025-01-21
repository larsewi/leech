#ifndef _LEECH_INSTANCE_H
#define _LEECH_INSTANCE_H

#include "table.h"

typedef struct LCH_Instance LCH_Instance;

/**
 * @brief Load the Leech instance from configuration file
 * @param work_dir Path to leech working directory
 * @return The instance or NULL in case of Failure
 * @note Leech will look for leech.json within the leech working directory
 */
LCH_Instance *LCH_InstanceLoad(const char *work_dir);

/**
 * @brief Destroy the instance
 * @param instance The instance
 */
void LCH_InstanceDestroy(void *instance);

/**
 * @brief Get the table definition given a unique table identifier
 * @param instance The instance
 * @param table_id Unique table identifier
 * @return The table definition or NULL in case of failure
 */
const LCH_TableInfo *LCH_InstanceGetTable(const LCH_Instance *instance,
                                          const char *table_id);

/**
 * @brief Get a list of all table definitions
 * @param instance The instance
 * @return A list of all table definitions or NULL in case of failure
 */
const LCH_List *LCH_InstanceGetTables(const LCH_Instance *instance);

/**
 * @brief Get the leech working directory
 * @param instance The instance
 * @return The leech working directory
 */
const char *LCH_InstanceGetWorkDirectory(const LCH_Instance *instance);

/**
 * @brief Get the preferred chain length
 * @param instance The instance
 * @return The preferred chain length
 * @note The preferred chain length is used to determine what blocks to prune on
 *       a call to LCH_Purge()
 */
size_t LCH_InstanceGetPreferredChainLength(const LCH_Instance *instance);

/**
 * @brief Whether or not JSON should be pretty printed
 * @param instance The instance
 * @return True if JSON should be pretty printed
 * @note Pretty print makes block more human readable, but takes up unecessary
 *       disk space
 */
bool LCH_InstanceShouldPrettyPrint(const LCH_Instance *instance);

/**
 * @brief Whether or not leech should auto purge
 * @param instance The instance
 * @return True if leech should auto purge
 * @note With auto purge enabled, leech should prune old blocks after each
 *       commit
 */
bool LCH_InstanceShouldAutoPurge(const LCH_Instance *instance);

#endif  // _LEECH_INSTANCE_H
