#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <math.h>

#include "buffer.h"
#include "csv.h"
#include "list.h"
#include "logger.h"
#include "sha1.h"

/******************************************************************************/

static bool IndicesOfFieldsInHeader(size_t *const indices,
                                    const LCH_List *const fields,
                                    const LCH_List *const header) {
  assert(indices != NULL);
  assert(fields != NULL);
  assert(header != NULL);

  const size_t header_len = LCH_ListLength(header);

  const size_t num_fields = LCH_ListLength(fields);
  for (size_t i = 0; i < num_fields; i++) {
    const LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(fields, i);
    if (field == NULL) {
      return false;
    }

    const size_t index =
        LCH_ListIndex(header, field, (LCH_CompareFn)LCH_BufferCompare);
    if (index >= header_len) {
      LCH_LOG_ERROR("Field '%s' not found in table header");
      return false;
    }
    indices[i] = index;
  }
  return true;
}

/******************************************************************************/

static LCH_List *FieldsInRecordAtIndices(const size_t *const indices,
                                         const size_t num_indices,
                                         const LCH_List *const record) {
  LCH_List *fields = LCH_ListCreate();
  if (fields == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < num_indices; i++) {
    const size_t index = indices[i];
    LCH_Buffer *const field = (LCH_Buffer *)LCH_ListGet(record, index);
    assert(field != NULL);

    if (!LCH_ListAppend(fields, field, NULL)) {
      LCH_ListDestroy(fields);
      return NULL;
    }
  }

  return fields;
}

/******************************************************************************/

LCH_Json *LCH_TableToJsonObject(const LCH_List *const table,
                                const LCH_List *const primary_fields,
                                const LCH_List *const subsidiary_fields) {
  const size_t num_records = LCH_ListLength(table);
  assert(num_records >= 1);  // Require at least a table header
  const LCH_List *const header = (LCH_List *)LCH_ListGet(table, 0);

  const size_t num_primary = LCH_ListLength(primary_fields);
  const size_t num_subsidiary = LCH_ListLength(subsidiary_fields);
  assert(num_primary > 0);  // Require at least one primary field
  assert(LCH_ListLength(header) == num_primary + num_subsidiary);

  size_t primary_indices[num_primary];
  if (!IndicesOfFieldsInHeader(primary_indices, primary_fields, header)) {
    return NULL;
  }

  size_t subsidiary_indices[num_subsidiary];
  if (!IndicesOfFieldsInHeader(subsidiary_indices, subsidiary_fields, header)) {
    return NULL;
  }

  LCH_Json *const object = LCH_JsonObjectCreate();
  if (object == NULL) {
    return NULL;
  }

  for (size_t i = 1; i < num_records; i++) {
    const LCH_List *const record = (LCH_List *)LCH_ListGet(table, i);
    assert(record != NULL);
    assert(LCH_ListLength(header) == LCH_ListLength(record));

    // Create key from primary fields
    LCH_Buffer *key = NULL;
    {
      LCH_List *const list =
          FieldsInRecordAtIndices(primary_indices, num_primary, record);
      if (list == NULL) {
        LCH_JsonDestroy(object);
        return NULL;
      }

      if (!LCH_CSVComposeRecord(&key, list)) {
        LCH_ListDestroy(list);
        LCH_JsonDestroy(object);
        return NULL;
      }

      LCH_ListDestroy(list);
    }

    // Create value from subsidiary fields
    LCH_Buffer *value = NULL;
    {
      LCH_List *const list =
          FieldsInRecordAtIndices(subsidiary_indices, num_subsidiary, record);
      if (list == NULL) {
        LCH_BufferDestroy(key);
        LCH_JsonDestroy(object);
        return NULL;
      }

      if (!LCH_CSVComposeRecord(&value, list)) {
        LCH_ListDestroy(list);
        LCH_BufferDestroy(key);
        LCH_JsonDestroy(object);
        return NULL;
      }
      LCH_ListDestroy(list);
    }

    assert(key != NULL);
    assert(value != NULL);
    if (!LCH_JsonObjectSetString(object, key, value)) {
      LCH_BufferDestroy(value);
      LCH_BufferDestroy(key);
      LCH_JsonDestroy(object);
      return NULL;
    }
    LCH_BufferDestroy(key);
  }

  return object;
}

/******************************************************************************/

bool LCH_MessageDigest(const unsigned char *const message, const size_t length,
                       LCH_Buffer *const digest_hex) {
  SHA1Context ctx;
  int ret = SHA1Reset(&ctx);
  if (ret != shaSuccess) {
    return false;
  }

  ret = SHA1Input(&ctx, message, length);
  if (ret != shaSuccess) {
    return false;
  }

  uint8_t tmp[SHA1HashSize];
  ret = SHA1Result(&ctx, tmp);
  if (ret != shaSuccess) {
    return false;
  }

  LCH_Buffer *const digest_bytes = LCH_BufferCreate();
  if (digest_bytes == NULL) {
    return false;
  }

  size_t offset;
  if (!LCH_BufferAllocate(digest_bytes, SHA1HashSize, &offset)) {
    LCH_BufferDestroy(digest_bytes);
    return false;
  }
  LCH_BufferSet(digest_bytes, offset, tmp, SHA1HashSize);

  if (!LCH_BufferBytesToHex(digest_hex, digest_bytes)) {
    LCH_BufferDestroy(digest_bytes);
    return false;
  }

  LCH_BufferDestroy(digest_bytes);
  return true;
}

/******************************************************************************/

bool LCH_ListInsertBufferDuplicate(LCH_List *const list, const size_t index,
                                   const LCH_Buffer *const buffer) {
  LCH_Buffer *const duplicate = LCH_BufferDuplicate(buffer);
  if (duplicate == NULL) {
    return false;
  }

  if (!LCH_ListInsert(list, index, duplicate, LCH_BufferDestroy)) {
    LCH_BufferDestroy(duplicate);
    return false;
  }
  return true;
}

/******************************************************************************/

bool LCH_ListAppendBufferDuplicate(LCH_List *const list,
                                   const LCH_Buffer *const buffer) {
  LCH_Buffer *const duplicate = LCH_BufferDuplicate(buffer);
  if (duplicate == NULL) {
    return false;
  }

  return LCH_ListAppend(list, duplicate, LCH_BufferDestroy);
}

/******************************************************************************/

bool LCH_DoubleToSize(const double number, size_t *const size) {
  assert(size != NULL);

  const char *const msg = "Failed to cast double to size_t";
  if (isfinite(number) == 0) {
    LCH_LOG_ERROR("%s: Number is not finite", msg, SIZE_MAX);
    return false;
  }
  if (number > SIZE_MAX) {
    LCH_LOG_ERROR("%s: Out of bounds for size_t (%g > %zu)", msg, number,
                  SIZE_MAX);
    return false;
  }
  if (number < 0) {
    LCH_LOG_ERROR("%s: Out of bound for size_t (%g < 0)", msg, number);
    return false;
  }
  *size = number;
  return true;
}
