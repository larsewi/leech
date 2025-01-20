#ifndef _LEECH_DELTA_H
#define _LEECH_DELTA_H

#include "json.h"

/**
 * @brief Create a patch between two table states
 * @param table_id The unique table identifier
 * @param type The delta type (either "delta" or "rebase")
 * @param new_state The current state of the table
 * @param old_state The previous state of the table
 * @return A patch in the form of a JSON object or NULL in case of failure
 * @note The type argument has no real effect on the resulting patch, it's just
 *       meta data to tell the LCH_Patch function how to interpret the data. To
 *       make a proper rebase patch, you need to pass an empty table as the old
 *       state argument, instead of the previous state
 */
LCH_Json *LCH_DeltaCreate(const char *table_id, const char *type,
                          const LCH_Json *new_state, const LCH_Json *old_state);

/**
 * @brief Get the unqiue table identifier of the delta
 * @param delta The patch as a JSON object
 * @return The unique table identifier or NULL in case of failure
 */
const char *LCH_DeltaGetTableId(const LCH_Json *delta);

/**
 * @brief Get the patch type
 * @param delta The patch as a JSON structure
 * @return The type or NULL in case of failure
 */
const char *LCH_DeltaGetType(const LCH_Json *delta);

/**
 * @brief Get the insert operations of a patch
 * @param delta The patch as a JSON object
 * @return The insert operations as a JSON object or NULL in case of failure
 */
const LCH_Json *LCH_DeltaGetInserts(const LCH_Json *delta);

/**
 * @brief Get the delete operations of a patch
 * @param delta The patch as a JSON object
 * @return The delete operations as a JSON object or NULL in case of failure
 */
const LCH_Json *LCH_DeltaGetDeletes(const LCH_Json *delta);

/**
 * @brief Get the update operations of a patch
 * @param delta The patch as a JSON object
 * @return The update operations as a JSON object or NULL in case of failure
 */
const LCH_Json *LCH_DeltaGetUpdates(const LCH_Json *delta);

/**
 * @brief Get the number operations of in a patch
 * @param delta The patch as a JSON object
 * @param num_inserts A pointer to the variable in which to store the total
 *                    number of insert operations or NULL if you don't care
 * @param num_deletes A pointer to the variable in which to store the total
 *                    number of delete operations or NULL if you don't care
 * @param num_inserts A pointer to the variable in which to store the total
 *                    number of update operations or NULL if you don't care
 * @return False in case of failure
 */
bool LCH_DeltaGetNumOperations(const LCH_Json *delta, size_t *num_inserts,
                               size_t *num_deletes, size_t *num_updates);

/**
 * @brief Merge two patches
 * @param parent The patch from the parent block
 * @param child The patch from the child block
 * @note The insert-, delete-, & update operations are removed from the child
 *       patch during the merge. Both child- & parent patches are mutated
 */
bool LCH_DeltaMerge(const LCH_Json *parent, LCH_Json *child);

#endif  // _LEECH_DELTA_H
