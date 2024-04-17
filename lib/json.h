#ifndef _LEECH_JSON_H
#define _LEECH_JSON_H

#include <stdbool.h>
#include <stdlib.h>

#include "buffer.h"
#include "list.h"

typedef struct LCH_Json LCH_Json;

/****************************************************************************/

typedef enum {
  LCH_JSON_TYPE_NULL = 0,
  LCH_JSON_TYPE_TRUE,
  LCH_JSON_TYPE_FALSE,
  LCH_JSON_TYPE_STRING,
  LCH_JSON_TYPE_NUMBER,
  LCH_JSON_TYPE_ARRAY,
  LCH_JSON_TYPE_OBJECT,
} LCH_JsonType;

/**
 * @brief Get JSON type as an LCH_JsonGetType enumerator.
 * @param json Element to get type from.
 * @return LCH_JsonGetType enumerator representation of type.
 */
LCH_JsonType LCH_JsonGetType(const LCH_Json *json);

/**
 * @brief Get JSON type as a string.
 * @param json Element to get type from.
 * @return Pointer to static string representing the type.
 * @note The returned value is one of; "null", "true", "false", "string",
 *       "number", "array" or "object".
 */
const char *LCH_JsonGetTypeAsString(const LCH_Json *json);

/****************************************************************************/

/**
 * @brief Check if JSON element is of type null.
 * @param json Element to check type of.
 * @return True if element is of type null.
 */
bool LCH_JsonIsNull(const LCH_Json *json);

/**
 * @brief Check if JSON element is of type true.
 * @param json Element to check type of.
 * @return True if element is of type true.
 */
bool LCH_JsonIsTrue(const LCH_Json *json);

/**
 * @brief Check if JSON element is of type false.
 * @param json Element to check type of.
 * @return True if element is of type false.
 */
bool LCH_JsonIsFalse(const LCH_Json *json);

/**
 * @brief Check if JSON element is of type string.
 * @param json Element to check type of.
 * @return True if element is of type string.
 */
bool LCH_JsonIsString(const LCH_Json *json);

/**
 * @brief Check if JSON element is of type number.
 * @param json Element to check type of.
 * @return True if element is of type number.
 */
bool LCH_JsonIsNumber(const LCH_Json *json);

/**
 * @brief Check if JSON element is of type object.
 * @param json Element to check type of.
 * @return True if element is of type object.
 */
bool LCH_JsonIsObject(const LCH_Json *json);

/**
 * @brief Check if JSON element is of type array.
 * @param json Element to check type of.
 * @return True if element is of type array.
 */
bool LCH_JsonIsArray(const LCH_Json *json);

/****************************************************************************/

/**
 * @brief Check if a JSON object's child element is of type null.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type null.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsNull(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Check if a JSON object's child element is of type true.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type true.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsTrue(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Check if a JSON object's child element is of type false.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type false.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsFalse(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Check if a JSON object's child element is of type string.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type string.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsString(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Check if a JSON object's child element is of type number.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type number.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsNumber(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Check if a JSON object's child element is of type object.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type object.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsObject(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Check if a JSON object's child element is of type array.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type array.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsArray(const LCH_Json *json, const LCH_Buffer *key);

/****************************************************************************/

/**
 * @brief Check if a JSON array's child element is of type null.
 * @param json Array with child to check type of.
 * @param index Index identifying the child.
 * @return True if array's child is of type null.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonArrayChildIsNull(const LCH_Json *json, size_t index);

/**
 * @brief Check if a JSON array's child element is of type true.
 * @param json Array with child to check type of.
 * @param index Index identifying the child.
 * @return True if array's child is of type true.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonArrayChildIsTrue(const LCH_Json *json, size_t index);

/**
 * @brief Check if a JSON array's child element is of type false.
 * @param json Array with child to check type of.
 * @param index Index identifying the child.
 * @return True if array's child is of type false.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonArrayChildIsFalse(const LCH_Json *json, size_t index);

/**
 * @brief Check if a JSON array's child element is of type string.
 * @param json Array with child to check type of.
 * @param index Index identifying the child.
 * @return True if array's child is of type string.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonArrayChildIsString(const LCH_Json *json, size_t index);

/**
 * @brief Check if a JSON array's child element is of type number.
 * @param json Array with child to check type of.
 * @param index Index identifying the child.
 * @return True if array's child is of type number.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonArrayChildIsNumber(const LCH_Json *json, size_t index);

/**
 * @brief Check if a JSON array's child element is of type object.
 * @param json Array with child to check type of.
 * @param index Index identifying the child.
 * @return True if array's child is of type object.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonArrayChildIsObject(const LCH_Json *json, size_t index);

/**
 * @brief Check if a JSON array's child element is of type array.
 * @param json Array with child to check type of.
 * @param index Index identifying the child.
 * @return True if array's child is of type array.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonArrayChildIsArray(const LCH_Json *json, size_t index);

/****************************************************************************/

/**
 * @brief Create JSON element of type null.
 * @return Element or NULL-pointer in case of memory error.
 */
LCH_Json *LCH_JsonNullCreate();

/**
 * @brief Create JSON element of type true.
 * @return Element or NULL-pointer in case of memory error.
 */
LCH_Json *LCH_JsonTrueCreate();

/**
 * @brief Create JSON element of type false.
 * @return Element or NULL-pointer in case of memory error.
 */
LCH_Json *LCH_JsonFalseCreate();

/**
 * @brief Create JSON element of type string.
 * @param str String value of element.
 * @return Element or NULL-pointer in case of memory error.
 * @note This function takes ownership of passed string argument.
 */
LCH_Json *LCH_JsonStringCreate(LCH_Buffer *str);

/**
 * @brief Create JSON element of type number.
 * @param num Number value of element.
 * @return Element or NULL-pointer in case of memory error.
 */
LCH_Json *LCH_JsonNumberCreate(double num);

/**
 * @brief Create JSON element of type object.
 * @return Element or NULL-pointer in case of memory error.
 */
LCH_Json *LCH_JsonObjectCreate();

/**
 * @brief Create JSON element of type array.
 * @return Element or NULL-pointer in case of memory error.
 */
LCH_Json *LCH_JsonArrayCreate();

/****************************************************************************/

/**
 * @brief Get number value assiciated with JSON number.
 * @param json Element to get number from.
 * @warning This function assumes passed JSON element is of type number.
 */
double LCH_JsonNumberGet(const LCH_Json *json);

/**
 * @brief Get child element in JSON object.
 * @param json Object to get element from.
 * @param key Key of element to get.
 * @return Child element with key.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type object.
 */
const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Get child element from JSON array.
 * @param json Array to get element from.
 * @param index Index of element to get.
 * @return Child element at index.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type array.
 */
const LCH_Json *LCH_JsonArrayGet(const LCH_Json *json, size_t index);

/**
 * @brief Get the string value of JSON element.
 * @return String value of element.
 * @warning This function makes the assumption that the element is of type
 *          string.
 */
const LCH_Buffer *LCH_JsonStringGet(const LCH_Json *json);

/**
 * @brief Get the string value from child element in JSON object.
 * @param json Object to get element from.
 * @param key Key of element to get.
 * @return String value of child element.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type object.
 */
const LCH_Buffer *LCH_JsonObjectGetString(const LCH_Json *json,
                                          const LCH_Buffer *key);

/**
 * @brief Get the string value from child element in JSON array.
 * @param json Array to get element from.
 * @param index Index of element to get.
 * @return String value of child element.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type array.
 */
const LCH_Buffer *LCH_JsonArrayGetString(const LCH_Json *json, size_t index);

/**
 * @brief Get JSON object from child element in JSON object.
 * @param json Object to get element from.
 * @param key Key of element to get.
 * @return Child object.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type object.
 */
const LCH_Json *LCH_JsonObjectGetObject(const LCH_Json *json,
                                        const LCH_Buffer *key);

/**
 * @brief Get JSON number from child element in JSON object.
 * @param json Object to get element from.
 * @param key Key of element to get.
 * @param number Pointer to double in which the number should be stored.
 * @return True on success, otherwise false.
 * @note In case of failure, the number parameter remains untouched.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type object.
 */
bool LCH_JsonObjectGetNumber(const LCH_Json *json, const LCH_Buffer *key,
                             double *number);

/**
 * @brief Get JSON object from child element in JSON array.
 * @param json Array to get element from.
 * @param index Index of element to get.
 * @return Child object.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type array.
 */
const LCH_Json *LCH_JsonArrayGetObject(const LCH_Json *json, size_t index);

/**
 * @brief Get JSON array from child element in JSON object.
 * @param json Object to get element from.
 * @param key Key of element to get.
 * @return Child object.
 * @warning This function makes the assumption that the passed JSON element is
 *          of type object.
 */
const LCH_Json *LCH_JsonObjectGetArray(const LCH_Json *json,
                                       const LCH_Buffer *key);

/****************************************************************************/

/**
 * @brief Create/update entry in JSON object.
 * @param json JSON object to create/update entry in.
 * @param key Key of entry.
 * @param value Value of entry.
 * @return True on success, false in case of memory errors.
 * @note This function takes ownership of passed value argument.
 * @warning This function assumes the passed JSON element is of type object.
 */
bool LCH_JsonObjectSet(const LCH_Json *json, const LCH_Buffer *key,
                       LCH_Json *value);

/**
 * @brief Create/update string entry in JSON object.
 * @param json JSON object to create/update entry in.
 * @param key Key of entry.
 * @param value String value of entry.
 * @return True on success, false in case of memory errors.
 * @note This function takes ownership of passed value argument.
 * @warning This function assumes the passed JSON element is of type object.
 */
bool LCH_JsonObjectSetString(const LCH_Json *json, const LCH_Buffer *key,
                             LCH_Buffer *value);

/**
 * @brief Create/update string entry in JSON object.
 * @param json JSON object to create/update entry in.
 * @param key Key of entry.
 * @param value String value of entry.
 * @return True on success, false in case of memory errors.
 * @note This function does not take ownership of passed value argument.
 * @warning This function assumes the passed JSON element is of type object.
 */
bool LCH_JsonObjectSetStringDuplicate(const LCH_Json *json,
                                      const LCH_Buffer *key,
                                      const LCH_Buffer *value);

/**
 * @brief Create/update number entry in JSON object.
 * @param json JSON object to create/update entry in.
 * @param key Key of entry.
 * @param value String value of entry.
 * @return True on success, false in case of memory errors.
 * @warning This function assumes the passed JSON element is of type object.
 */
bool LCH_JsonObjectSetNumber(const LCH_Json *json, const LCH_Buffer *key,
                             double number);

/****************************************************************************/

/**
 * @brief Append entry to end of JSON array.
 * @param json JSON array to append entry to.
 * @param value Value to append.
 * @return True on succes, false in case of memory errors.
 * @note This function takes ownership of passed value argument.
 * @warning This function assumes the passed JSON element is of type array.
 */
bool LCH_JsonArrayAppend(const LCH_Json *json, LCH_Json *value);

/**
 * @brief Append string element to JSON array.
 * @param json JSON array to append element to.
 * @param value String value of entry.
 * @return True on success, false in case of memory errors.
 * @note This function takes ownership of passed value argument.
 * @warning This function assumes the passed JSON element is of type array.
 */
bool LCH_JsonArrayAppendString(const LCH_Json *json, LCH_Buffer *value);

/****************************************************************************/

/**
 * @brief Remove element with key from JSON object.
 * @param json JSON object to remove element from.
 * @param key Key of entry to remove.
 * @return Value of removed entry.
 * @note This function relieves ownership of returned value and it does not
 *       error.
 * @warning This function assumes the passed JSON element is of type object.
 */
LCH_Json *LCH_JsonObjectRemove(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Remove element at index from JSON array.
 * @param json JSON object to remove element from.
 * @param index Index of element to remove.
 * @return Removed element.
 * @note This function relieves ownership of returned value and it does not
 *       error.
 * @warning This function assumes the passed JSON element is of type array.
 */
LCH_Json *LCH_JsonArrayRemove(const LCH_Json *json, size_t index);

/**
 * @brief Remove JSON object with key from JSON object.
 * @param json JSON object to remove entry from.
 * @param key Key of entry to remove.
 * @return Value of removed entry, or NULL if child is not of type object.
 * @note This function relieves ownership of returned value.
 * @warning This function assumes the passed JSON element is of type object.
 */
LCH_Json *LCH_JsonObjectRemoveObject(const LCH_Json *json,
                                     const LCH_Buffer *key);

/**
 * @brief Remove JSON object at index from JSON array.
 * @param json JSON array to remove element from.
 * @param index Index of element to remove.
 * @return Value of removed element, or NULL if child is not of type object.
 * @note This function relieves ownership of returned value.
 * @warning This function assumes the passed JSON element is of type array.
 */
LCH_Json *LCH_JsonArrayRemoveObject(const LCH_Json *json, size_t index);

/**
 * @brief Remove JSON array with key from JSON object.
 * @param json JSON object to remove entry from.
 * @param key Key of entry to remove.
 * @return Value of removed entry, or NULL if child is not of type array.
 * @note This function relieves ownership of returned value.
 * @warning This function assumes the passed JSON element is of type object.
 */
LCH_Json *LCH_JsonObjectRemoveArray(const LCH_Json *json,
                                    const LCH_Buffer *key);

/**
 * @brief Remove JSON array at index from JSON array.
 * @param json JSON array to remove element from.
 * @param index Index of element to remove.
 * @return Value of removed element, or NULL if child is not of type array.
 * @note This function relieves ownership of returned value.
 * @warning This function assumes the passed JSON element is of type array.
 */
LCH_Json *LCH_JsonArrayRemoveArray(const LCH_Json *json, size_t index);

/****************************************************************************/

/**
 * @brief Get a list of existing keys in a JSON object.
 * @param json Object to get keys from.
 * @return List of existing keys or NULL in case of memory errors.
 * @note Returned list must be free'd with a call to LCH_ListDestroy().
 * @warning This function assumes the passed JSON element is of type object.
 */
LCH_List *LCH_JsonObjectGetKeys(const LCH_Json *json);

/**
 * @brief Check for existing key in JSON object.
 * @param json Object to check.
 * @param key Key to check for.
 * @return True of key exists in object.
 * @warning This function assumes the passed JSON element is of type object.
 */
bool LCH_JsonObjectHasKey(const LCH_Json *json, const LCH_Buffer *key);

/**
 * @brief Get number of entries in JSON object.
 * @param json Object to get number of entries from.
 * @return Number of entries in object.
 * @warning This function assumes the passed JSON element is of type object.
 */
size_t LCH_JsonObjectLength(const LCH_Json *json);

/**
 * @brief Get number of elements in JSON array.
 * @param json Array to get number of elements from.
 * @return Number of elements in array.
 * @warning This function assumes the passed JSON element is of type array.
 */
size_t LCH_JsonArrayLength(const LCH_Json *json);

/****************************************************************************/

/**
 * @brief Get a copy of all key-value pairs of the left operand where the key
 *        is not present in the right operand.
 * @param left Left operand JSON object.
 * @param right Right operand JSON object.
 * @return Result of set minus operation or NULL in case of memory errors.
 * @note returned JSON object must be free'd with a call to LCH_JsonDestroy().
 * @warning This function assumes both passed JSON elements are of type object.
 */
LCH_Json *LCH_JsonObjectKeysSetMinus(const LCH_Json *left,
                                     const LCH_Json *right);

/**
 * @brief Get a copy of all key-value pairs where the key is present in both
 *        left and right operands, but where the value differs.
 * @param left Left operand JSON object.
 * @param right Right operand JSON object.
 * @return Result of set intersect operation on keys and set minus operation on
 *         values or NULL in case of memory errors.
 * @note returned JSON object must be free'd with a call to LCH_JsonDestroy().
 * @warning This function assumes both passed JSON elements are of type object.
 */
LCH_Json *LCH_JsonObjectKeysSetIntersectAndValuesSetMinus(
    const LCH_Json *left, const LCH_Json *right);

/****************************************************************************/

/**
 * @brief Get a deep copy of JSON element.
 * @param json JSON element to copy.
 * @return Deep copy or NULL in case of memory errors.
 */
LCH_Json *LCH_JsonCopy(const LCH_Json *json);

/****************************************************************************/

/**
 * @brief Check if two JSON elements are equal.
 * @param left Left operand JSON element.
 * @param right Right operand JSON element.
 * @return True if equal, otherwise false.
 */
bool LCH_JsonEqual(const LCH_Json *left, const LCH_Json *right);

/****************************************************************************/

/**
 * @brief Parse JSON formatted string.
 * @param str JSON formatted string.
 * @param len Length of JSON formatted string (excluding the optional
 *            terminating null-byte)
 * @return Parsed JSON object.
 * @note Unlike other JSON parser, here JSON strings do not have to be
 *       NULL-terminated nor do they have to be ASCII.
 */
LCH_Json *LCH_JsonParse(const char *str, size_t len);

/**
 * @brief Parse JSON formatted file.
 * @param filename Path to JSON formatted file.
 * @return Parsed JSON object.
 * @note Unlike other JSON parser, here JSON strings do not have to be
 *       NULL-terminated nor do they have to be ASCII.
 */
LCH_Json *LCH_JsonParseFile(const char *filename);

/****************************************************************************/

/**
 * @brief Compose JSON element into JSON formatted string.
 * @param json JSON element to compose.
 * @return Buffer containing JSON formatted string, or NULL in case of errors.
 * @note Unlike other JSON composers, the returned buffer can contain non ASCII
 *       characters.
 */
LCH_Buffer *LCH_JsonCompose(const LCH_Json *json, bool pretty);

/**
 * @brief Compose JSON element into JSON formatted file.
 * @param json JSON element to compose.
 * @param filename path to file.
 * @return True on success, otherwise false.
 * @note Unlike other JSON composers, the returned buffer can contain non ASCII
 *       characters.
 */
bool LCH_JsonComposeFile(const LCH_Json *json, const char *filename,
                         bool pretty);

/****************************************************************************/

/**
 * @brief Recusively destroy JSON element.
 * @param json Pointer to JSON element.
 */
void LCH_JsonDestroy(void *json);

#endif  // _LEECH_JSON_H
