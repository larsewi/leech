#ifndef _LEECH_JSON_H
#define _LEECH_JSON_H

#include <stdbool.h>
#include <stdlib.h>

#include "dict.h"
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
 * @brief Get JSON type as an enum.
 * @param json Element to get type from.
 * @return Enum representation of type.
 */
LCH_JsonType LCH_JsonGetType(const LCH_Json *json);

/**
 * @brief Get JSON type as a string.
 * @param json Element to get type from.
 * @return String representation of type.
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
bool LCH_JsonObjectChildIsNull(const LCH_Json *json, const char *key);

/**
 * @brief Check if a JSON object's child element is of type true.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type true.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsTrue(const LCH_Json *json, const char *key);

/**
 * @brief Check if a JSON object's child element is of type false.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type false.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsFalse(const LCH_Json *json, const char *key);

/**
 * @brief Check if a JSON object's child element is of type string.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type string.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsString(const LCH_Json *json, const char *key);

/**
 * @brief Check if a JSON object's child element is of type number.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type number.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsNumber(const LCH_Json *json, const char *key);

/**
 * @brief Check if a JSON object's child element is of type object.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type object.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsObject(const LCH_Json *json, const char *key);

/**
 * @brief Check if a JSON object's child element is of type array.
 * @param json Object with child to check type of.
 * @param key Key identifying the child.
 * @return True if object's child is of type array.
 * @warning This function makes the assumtion that the child exists.
 */
bool LCH_JsonObjectChildIsArray(const LCH_Json *json, const char *key);

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
 */
LCH_Json *LCH_JsonStringCreate(char *str);

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

double LCH_JsonGetNumber(const LCH_Json *json);

/**
 * @brief Get child element in JSON object.
 * @param json Object to get element from.
 * @param key Key of child element to get.
 * @return Child element.
 * @warning This function makes the assumption that the element is of type
 *          object and key exists in it.
 */
const LCH_Json *LCH_JsonObjectGet(const LCH_Json *json, const char *key);

const LCH_Json *LCH_JsonArrayGet(const LCH_Json *json, size_t index);

/**
 * @brief Get the string value of JSON element.
 * @return String value of element.
 * @warning This function makes the assumption that the element is of type
 *          string.
 */
const char *LCH_JsonStringGetString(const LCH_Json *json);

const char *LCH_JsonObjectGetString(const LCH_Json *json, const char *key);

const char *LCH_JsonArrayGetString(const LCH_Json *json, size_t index);

const LCH_Json *LCH_JsonObjectGetObject(const LCH_Json *json, const char *key);

const LCH_Json *LCH_JsonArrayGetObject(const LCH_Json *json, size_t index);

const LCH_Json *LCH_JsonObjectGetArray(const LCH_Json *json, const char *key);

/****************************************************************************/

bool LCH_JsonObjectSet(const LCH_Json *json, const char *key, LCH_Json *value);

bool LCH_JsonObjectSetString(const LCH_Json *json, const char *key,
                             char *value);

bool LCH_JsonObjectSetStringDuplicate(const LCH_Json *json, const char *key,
                                      const char *value);

bool LCH_JsonObjectSetNumber(const LCH_Json *json, const char *key,
                             double number);

bool LCH_JsonArrayAppend(const LCH_Json *json, LCH_Json *value);

/****************************************************************************/

LCH_Json *LCH_JsonObjectRemove(const LCH_Json *json, const char *key);

LCH_Json *LCH_JsonArrayRemove(const LCH_Json *json, size_t index);

LCH_Json *LCH_JsonObjectRemoveObject(const LCH_Json *json, const char *key);

LCH_Json *LCH_JsonArrayRemoveObject(const LCH_Json *json, size_t index);

LCH_Json *LCH_JsonObjectRemoveArray(const LCH_Json *json, const char *key);

LCH_Json *LCH_JsonArrayRemoveArray(const LCH_Json *json, size_t index);

/****************************************************************************/

/**
 * @brief Get a list of existing keys in a JSON object.
 * @param json Object to get keys from.
 * @return List of existing keys.
 * @note Returned list must be free'd with a call to LCH_ListDestroy().
 * @warning This function makes the assumption that the element is of type
 *          object.
 */
LCH_List *LCH_JsonObjectGetKeys(const LCH_Json *json);

/**
 * @brief Check for existing key in JSON object.
 * @param json Object to check.
 * @param key Key to check for.
 * @return True of key exists in object.
 * @warning This function makes the assumption that the element is of type
 *          object.
 */
bool LCH_JsonObjectHasKey(const LCH_Json *json, const char *key);

size_t LCH_JsonObjectLength(const LCH_Json *json);

size_t LCH_JsonArrayLength(const LCH_Json *json);

/****************************************************************************/

LCH_Json *LCH_JsonObjectKeysSetMinus(const LCH_Json *a, const LCH_Json *b);

/**
 * @brief Get a copy of all key-value pairs in the left operand where the key is
 *        not present in the right operand.
 * @param a left operand
 * @param b right operand
 * @return new JSON object or NULL on error
 * @note returned JSON must be free'd with LCH_JsonDestroy
 */
LCH_Json *LCH_JsonObjectKeysSetIntersectAndValuesSetMinus(const LCH_Json *a,
                                                          const LCH_Json *b);

/****************************************************************************/

LCH_Json *LCH_JsonCopy(const LCH_Json *json);

/****************************************************************************/

bool LCH_JsonIsEqual(const LCH_Json *a, const LCH_Json *b);

/****************************************************************************/

LCH_Json *LCH_JsonParse(const char *str);

/****************************************************************************/

char *LCH_JsonCompose(const LCH_Json *json);

/****************************************************************************/

void LCH_JsonDestroy(void *json);

#endif  // _LEECH_JSON_H
