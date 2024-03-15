#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include "buffer.h"
#include "list.h"

/**
 * @brief Parse a CSV formatted string into a table.
 * @param csv CSV formatted string.
 * @param len Length of the CSV formatted string (excluding the optional
 *            terminating NULL-byte).
 * @return Two-dimentional list of buffers representing the table.
 * @note CSV string does not have to be NULL-terminated and fields do not have
 *       to be ASCII.
 */
LCH_List *LCH_CSVParseTable(const char *csv, size_t len);

/**
 * @brief Parse a CSV formatted string into a record.
 * @param csv CSV formatted string.
 * @param len Length of the CSV formatted string (excluding the optional
 *            terminating NULL-byte).
 * @return One-dimentional list of buffers representing the record.
 * @note CSV string does not have to be NULL-terminated and fields do not have
 *       to be ASCII.
 */
LCH_List *LCH_CSVParseRecord(const char *csv, size_t len);

/**
 * @brief Parse a CSV formatted string into a field.
 * @param csv CSV formatted string.
 * @param len Length of the CSV formatted string (excluding the optional
 *            terminating NULL-byte).
 * @return Buffer representing the field.
 * @note CSV string do not have to be NULL-terminated and fields do not have to
 *       be ASCII.
 */
LCH_Buffer *LCH_CSVParseField(const char *csv, size_t len);

/**
 * @brief Parse a CSV formatted file.
 * @param path Path to CSV file.
 * @return Two-dimentional list of buffers representing the table.
 * @note CSV string do not have to be NULL-terminated and fields do not have to
 *       be ASCII.
 */
LCH_List *LCH_CSVParseFile(const char *path);

/****************************************************************************/

/**
 * @brief Compose a CSV formatted string from a table.
 * @param csv Buffer to hold the resulting CSV string.
 * @param table Two-dimentional of buffers list representing the table.
 * @return True on success, otherwise false.
 * @note If the CSV buffer is NULL, then a new buffer is allocated on the heap.
 *       Buffer will remain untouched in case of failure.
 */
bool LCH_CSVComposeTable(LCH_Buffer **csv, const LCH_List *table);

/**
 * @brief Compose a CSV formatted string from a record (one dimentional list)
 * @param csv Buffer to hold the resulting CSV string.
 * @param record One-dimentional list of buffers representing the record.
 * @return True on success, otherwise false.
 * @note If the CSV buffer is NULL, then a new buffer is allocated on the heap.
 *       Buffer will remain untouched in case of failure.
 */
bool LCH_CSVComposeRecord(LCH_Buffer **csv, const LCH_List *record);

/**
 * @brief Compose a CSV formatted string from a record (one dimentional list)
 * @param csv Buffer to hold the resulting CSV string.
 * @param raw Raw string representing the field.
 * @param len Length of the CSV formatted string (excluding the optional
 *            terminating NULL-byte).
 * @return True on success, otherwise false.
 * @note If the CSV buffer is NULL, then a new buffer is allocated on the heap.
 *       Buffer will remain untouched in case of failure.
 */
bool LCH_CSVComposeField(LCH_Buffer **csv, const char *const raw, size_t len);

/**
 * @brief Compose a CSV file from a table.
 * @param table Table represented by a two-dimentional list of buffers.
 * @param path Path to CSV file (will be created/trunctated).
 * @return True on success, otherwise false.
 */
bool LCH_CSVComposeFile(const LCH_List *table, const char *path);

#endif  // _LEECH_CSV_H
