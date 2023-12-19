#ifndef _LEECH_INSTANCE_H
#define _LEECH_INSTANCE_H

#include "leech.h"
#include "json.h"

LCH_Json *LCH_InstanceLoad(const char work_dir);

/**
 * @brief Get table definition of a specific table
 * @param instance Leech instance
 * @param table_id unique table identifier
 * @return table definition
 */
const LCH_TableDefinition *LCH_InstanceGetTable(const LCH_Instance *instance,
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
