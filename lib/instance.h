#ifndef _LEECH_INSTANCE_H
#define _LEECH_INSTANCE_H

#include "table.h"

typedef struct LCH_Instance LCH_Instance;

/**
 * @brief Load Leech instance from configuration file.
 * @param work_dir Path to work directory.
 * @return Handle to instance.
 * @note Leech will look for leech.json within the work directory. The handle
 *       must be destroyed by a call to LCH_InstanceDestroy().
 */
LCH_Instance *LCH_InstanceLoad(const char *work_dir);

/**
 * @brief Destroy handle to Leech instance.
 */
void LCH_InstanceDestroy(void *instance);

/**
 * @brief Get table definition of a specific table
 * @param instance Leech instance
 * @param table_id unique table identifier
 * @return table definition
 */
const LCH_TableInfo *LCH_InstanceGetTable(const LCH_Instance *instance,
                                          const char *table_id);

/**
 * @brief Get a list of table definitions
 * @param instance Leech instance
 * @return list of table definitions
 */
const LCH_List *LCH_InstanceGetTables(const LCH_Instance *instance);

/**
 * @brief Get Leech work directory
 * @param instance Leech instance
 * @return Leech work directory
 */
const char *LCH_InstanceGetWorkDirectory(const LCH_Instance *instance);

#endif  // _LEECH_INSTANCE_H
