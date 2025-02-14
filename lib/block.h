#ifndef _LEECH_BLOCK_H
#define _LEECH_BLOCK_H

#include <stdbool.h>

#include "instance.h"
#include "json.h"

/**
 * @brief Create a new block
 * @param parent_id The block identifier of the parent block (usually the
 *                  currently latest block)
 * @param payload JSON list of deltas
 * @return Block as a JSON structure or NULL in case of failure
 */
LCH_Json *LCH_BlockCreate(const char *parent_id, LCH_Json *const payload);

/**
 * @brief Write a block to the disk and move the HEAD to point to this block
 * @param instance The leech instance
 * @param block The block
 * @return False in case of failure
 */
bool LCH_BlockStore(const LCH_Instance *const instance, const LCH_Json *block);

/**
 * @brief Load a block from disk
 * @param work_dir The leech working directory
 * @param block_id The block identifier
 * @return The block as a JSON structure or NULL in case of failure
 */
LCH_Json *LCH_BlockLoad(const char *work_dir, const char *block_id);

/**
 * @brief Get the protocol version of the block
 * @param block The block
 * @param version Pointer to the variable in which to store the version number
 * @return False in case of failure
 */
bool LCH_BlockGetVersion(const LCH_Json *block, size_t *version);

/**
 * @brief Get the parent block identifier of a block
 * @param block The block
 * @return The parent identifier or NULL in case of error
 */
const char *LCH_BlockGetParentId(const LCH_Json *block);

/**
 * @brief Check if block identifier matched that of the genisis block
 * @param block_id The block identifier
 * @return True if there is a match
 */
bool LCH_BlockIsGenisisId(const char *block_id);

/**
 * @brief Get the block payload
 * @param block The block
 * @return The payload (i.e., JSON list of deltas) or NULL in case of failure
 */
const LCH_Json *LCH_BlockGetPayload(const LCH_Json *block);

/**
 * @brief Remove the payload (i.e., list of deltas) from a block
 * @param block The block
 * @note The caller takes ownership of the payload
 */
LCH_Json *LCH_BlockRemovePayload(const LCH_Json *block);

/**
 * @brief Append payload to block
 * @param block The block
 * @param payload The payload
 * @return False in case of failure
 */
bool LCH_BlockAppendPayload(const LCH_Json *block, LCH_Json *payload);

/**
 * @brief Get the timestamp from whence the block was created
 * @param block The block
 * @param timestamp The variable in which to store the timestamp
 * @return False in case of failure
 */
bool LCH_BlockGetTimestamp(const LCH_Json *block, double *timestamp);

/**
 * @brief Get block identifier from partial hash
 * @param work_dir Leech work directory
 * @param argument Argument containing partial hash matching the start of the
 *                 identifier of an existing block
 * @return Full block identifier or NULL in case of ambiguous/unknown blocks or
 *         failure
 */
char *LCH_BlockIdFromArgument(const char *work_dir, const char *argument);

#endif  // _LEECH_BLOCK_H
