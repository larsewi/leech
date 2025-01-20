#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include "buffer.h"
#include "list.h"

/**
 * @brief Parse a CSV formatted string into a table
 * @param csv The CSV formatted string
 * @param len The length of the CSV formatted string (excluding the optional
 *            terminating null-byte)
 * @return Two-dimentional list of buffers or NULL in case of failure
 * @note The CSV string does not have to be null-byte terminated and fields do
 *       not have to be ASCII
 */
LCH_List *LCH_CSVParseTable(const char *csv, size_t len);

/**
 * @brief Parse a CSV formatted string into a record
 * @param csv The CSV formatted string
 * @param len The length of the CSV formatted string (excluding the optional
 *            terminating null-byte).
 * @return One-dimentional list of buffers representing the record or NULL in
 *         case of failure
 * @note The CSV string does not have to be null-byte terminated and fields do
 *       not have to be ASCII
 */
LCH_List *LCH_CSVParseRecord(const char *csv, size_t len);

/**
 * @brief Parse a CSV formatted string into a field
 * @param csv The CSV formatted string
 * @param len The length of the CSV formatted string (excluding the optional
 *            terminating null-byte).
 * @return Buffer representing the field or NULL in case of failure
 * @note The CSV string does not have to be null-byte terminated and fields do
 *       not have to be ASCII
 */
LCH_Buffer *LCH_CSVParseField(const char *csv, size_t len);

/**
 * @brief Parse a CSV formatted file into a table
 * @param path Path to CSV file
 * @return Two-dimentional list of buffers or NULL in case of failure
 * @note The CSV string does not have to be null-byte terminated and fields do
 *       not have to be ASCII
 */
LCH_List *LCH_CSVParseFile(const char *path);

/****************************************************************************/

/**
 * @brief Compose a CSV formatted string from a table
 * @param csv The buffer in which to write the resulting CSV string
 * @return Two-dimentional list of buffers or NULL in case of failure
 * @return False in case of failure
 * @note If the CSV buffer is NULL, then a new buffer is allocated on the heap
 */
bool LCH_CSVComposeTable(LCH_Buffer **csv, const LCH_List *table);

/**
 * @brief Compose a CSV formatted string from a record (one-dimentional list)
 * @param csv The buffer in which to write the resulting CSV string
 * @param record One-dimentional list of buffers representing the record
 * @return False in case of failure
 * @note If the CSV buffer is NULL, then a new buffer is allocated on the heap
 */
bool LCH_CSVComposeRecord(LCH_Buffer **csv, const LCH_List *record);

/**
 * @brief Compose a CSV formatted string from a field
 * @param csv The buffer in which to hold the resulting CSV string
 * @param raw A raw string representing the field
 * @param len The length of the raw string (excluding the optional terminating
 *            null-byte)
 * @return False in case of failure
 * @note If the CSV buffer is NULL, then a new buffer is allocated on the heap
 */
bool LCH_CSVComposeField(LCH_Buffer **csv, const char *const raw, size_t len);

/**
 * @brief Compose a CSV file from a table
 * @param table The table represented by a two-dimentional list of buffers
 * @param path Path to CSV file in which to write the resulting CSV
 * @return False in case of failure
 * @note The file will be created/trunctated
 */
bool LCH_CSVComposeFile(const LCH_List *table, const char *path);

#endif  // _LEECH_CSV_H
