#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "buffer.h"
#include "csv.h"
#include "definitions.h"
#include "leech.h"
#include "list.h"

LCH_List *LCH_SplitString(const char *str, const char *del) {
  LCH_List *list = LCH_ListCreate();
  size_t to, from = 0, len = strlen(str);
  bool is_delim, was_delim = true;

  for (to = 0; to < len; to++) {
    is_delim = strchr(del, str[to]) != NULL;
    if (is_delim) {
      if (was_delim) {
        continue;
      }
      assert(to > from);
      char *s = strndup(str + from, to - from);
      if (s == NULL) {
        LCH_LOG_ERROR("Failed to allocate memory during string split: %s",
                      strerror(errno));
        LCH_ListDestroy(list);
        return NULL;
      }
      if (!LCH_ListAppend(list, (void *)s, free)) {
        free(s);
        LCH_ListDestroy(list);
        return NULL;
      }
    } else {
      if (was_delim) {
        from = to;
      }
    }
    was_delim = is_delim;
  }

  if (from < to && !is_delim) {
    char *s = strndup(str + from, to - from);
    if (s == NULL) {
      LCH_ListDestroy(list);
      return NULL;
    }
    if (!LCH_ListAppend(list, (void *)s, free)) {
      LCH_ListDestroy(list);
      return NULL;
    }
  }

  return list;
}

/******************************************************************************/

static bool SplitStringSubstringMaybeAddElement(LCH_List *const lst,
                                                const char *const str,
                                                const size_t from,
                                                const size_t to) {
  if (from < to) {
    char *const item = strndup(str + from, to - from);
    if (item == NULL) {
      LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
      return false;
    }

    if (!LCH_ListAppend(lst, item, free)) {
      free(item);
      return false;
    }
  }

  return true;
}

LCH_List *LCH_SplitStringSubstring(const char *const str,
                                   const char *const substr) {
  assert(str != NULL);
  assert(substr != NULL);

  LCH_List *const lst = LCH_ListCreate();
  if (lst == NULL) {
    return NULL;
  }

  size_t from = 0, to;
  for (to = 0; str[to] != '\0'; to++) {
    size_t i;
    bool is_match = true;
    for (i = 0; substr[i] != '\0'; i++) {
      if (str[to + i] != substr[i]) {
        is_match = false;
        break;
      }
    }

    if (is_match) {
      if (!SplitStringSubstringMaybeAddElement(lst, str, from, to)) {
        LCH_ListDestroy(lst);
        return NULL;
      }
      to += i;
      from = to;
    }
  }

  if (!SplitStringSubstringMaybeAddElement(lst, str, from, to)) {
    LCH_ListDestroy(lst);
    return NULL;
  }

  return lst;
}

/******************************************************************************/

bool LCH_StringStartsWith(const char *const self, const char *const substr) {
  assert(self != NULL);
  assert(substr != NULL);

  size_t length = strlen(substr);
  for (size_t i = 0; i < length; i++) {
    if (self[i] != substr[i]) {
      return false;
    }
  }
  return true;
}

/******************************************************************************/

char *LCH_StringStrip(char *str, const char *charset) {
  assert(str != NULL);

  size_t start = 0, end = 0, cursor = 0;
  while (str[cursor] != '\0') {
    if (strchr(charset, str[cursor]) != NULL) {
      if (start == cursor) {
        ++start;
      }
    } else {
      end = cursor + 1;
    }
    ++cursor;
  }

  str = (char *)memmove((void *)str, (void *)(str + start), end - start);
  str[end - start] = '\0';
  return str;
}

/******************************************************************************/

bool LCH_FileSize(FILE *file, size_t *size) {
  if (fseek(file, 0, SEEK_END) != 0) {
    LCH_LOG_ERROR("Failed to seek to end of file: %s", strerror(errno));
    return false;
  }

  long pos = ftell(file);
  if (pos < 0) {
    LCH_LOG_ERROR("Failed to obtain the current file position indicator: %s",
                  strerror(errno));
    return false;
  }
  *size = (size_t)pos;

  if (fseek(file, 0, SEEK_SET) != 0) {
    LCH_LOG_ERROR("Failed to seek to start of file: %s", strerror(errno));
    return false;
  }

  return true;
}

/******************************************************************************/

bool LCH_FileExists(const char *const path) {
  struct stat sb = {0};
  return stat(path, &sb) == 0;
}

/******************************************************************************/

bool LCH_IsRegularFile(const char *const path) {
  struct stat sb = {0};
  return (stat(path, &sb) == 0) && ((sb.st_mode & S_IFMT) == S_IFREG);
}

/******************************************************************************/

bool LCH_IsDirectory(const char *const path) {
  struct stat sb = {0};
  return (stat(path, &sb) == 0) && ((sb.st_mode & S_IFMT) == S_IFDIR);
}

/******************************************************************************/

bool LCH_PathJoin(char *path, const size_t path_max, const size_t n_items,
                  ...) {
  assert(path_max >= 1);

  va_list ap;
  va_start(ap, n_items);

  size_t used = 0;
  bool truncated = false;
  for (size_t i = 0; i < n_items; i++) {
    if (i > 0) {
      if (path_max - used < 2) {
        truncated = true;
        break;
      }
      path[used++] = PATH_SEP;
    }

    char *const sub = va_arg(ap, char *);
    const size_t sub_len = strlen(sub);
    for (size_t j = 0; j < sub_len; j++) {
      if (path_max - used < 2) {
        truncated = true;
        break;
      }
      path[used++] = sub[j];
    }
  }

  va_end(ap);
  path[used] = '\0';

  if (truncated) {
    LCH_LOG_ERROR("Failed to join paths: Truncation error.");
    return false;
  }
  return true;
}

/******************************************************************************/

char *LCH_ReadFile(const char *const path, size_t *const length) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for reading: %s", path,
                  strerror(errno));
    return NULL;
  }

  char *buffer = NULL;
  size_t buffer_size = LCH_BUFFER_SIZE;
  size_t total_read = 0, bytes_read = 0;

  do {
    char *ptr = realloc(buffer, buffer_size);
    if (ptr == NULL) {
      LCH_LOG_ERROR(
          "Failed to reallocate (%zu bytes) memory for read buffer: %s",
          buffer_size, strerror(errno));
      free(buffer);
      fclose(file);
      return NULL;
    }

    buffer = ptr;
    bytes_read =
        fread(buffer + total_read, 1, buffer_size - total_read - 1, file);
    total_read += bytes_read;
    buffer_size *= 2;
  } while (bytes_read != 0);

  if (ferror(file)) {
    LCH_LOG_ERROR("Failed to read file '%s'.", path);
    free(buffer);
    fclose(file);
    return NULL;
  }
  fclose(file);

  if (length != NULL) {
    *length = total_read;
  }
  buffer[total_read] = '\0';

  return buffer;
}

/******************************************************************************/

bool LCH_WriteFile(const char *const path, const char *const str) {
  FILE *file = fopen(path, "w");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for writing: %s", path,
                  strerror(errno));
    return false;
  }

  if (fputs(str, file) == EOF) {
    LCH_LOG_ERROR("Failed to write to file '%s'.", path);
    fclose(file);
    return false;
  }

  fclose(file);
  return true;
}

/******************************************************************************/

static LCH_List *GetIndexOfFields(const LCH_List *const header,
                                  const LCH_List *const fields) {
  LCH_List *indices = LCH_ListCreate();
  if (indices == NULL) {
    return NULL;
  }

  const size_t header_len = LCH_ListLength(header);
  const size_t fields_len = LCH_ListLength(fields);
  if (fields_len > header_len) {
    LCH_LOG_ERROR(
        "Number of fields cannot exceed table header length (%zu > %zu).",
        fields_len, header_len);
    LCH_ListDestroy(indices);
    return NULL;
  }

  for (size_t i = 0; i < fields_len; i++) {
    const char *const field = LCH_ListGet(fields, i);

    size_t index = LCH_ListIndex(header, field,
                                 (int (*)(const void *, const void *))strcmp);

    if (index >= header_len) {
      LCH_Buffer *const buffer = LCH_CSVComposeRecord(header);
      LCH_LOG_ERROR("Field '%s' not found in table header '%s'.", field,
                    LCH_BufferGet(buffer, 0));
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(indices);
      return NULL;
    }

    if (!LCH_ListAppend(indices, (void *)index, NULL)) {
      LCH_ListDestroy(indices);
      return NULL;
    }
  }

  return indices;
}

static LCH_List *ExtractFieldsAtIndices(const LCH_List *const record,
                                        const LCH_List *const indices) {
  LCH_List *const fields = LCH_ListCreate();
  if (fields == NULL) {
    return NULL;
  }

  const size_t indices_len = LCH_ListLength(indices);
  const size_t record_len = LCH_ListLength(record);
  assert(record_len >= indices_len);

  for (size_t i = 0; i < indices_len; i++) {
    const size_t index = (size_t)LCH_ListGet(indices, i);
    assert(index < record_len);

    char *field = LCH_ListGet(record, index);
    assert(field != NULL);

    if (!LCH_ListAppend(fields, field, NULL)) {
      LCH_ListDestroy(fields);
      return NULL;
    }
  }

  return fields;
}

static char *ComposeFieldsAtIndices(const LCH_List *const record,
                                    const LCH_List *const indices) {
  LCH_List *const extracted = ExtractFieldsAtIndices(record, indices);
  if (extracted == NULL) {
    LCH_Buffer *const ind = LCH_CSVComposeRecord(indices);
    LCH_Buffer *const rec = LCH_CSVComposeRecord(record);
    LCH_LOG_ERROR("Failed to extract fields at indices '%s' from record '%s'.",
                  LCH_BufferGet(ind, 0), LCH_BufferGet(rec, 0));
    LCH_BufferDestroy(ind);
    LCH_BufferDestroy(rec);
    return NULL;
  }

  LCH_Buffer *const buffer = LCH_CSVComposeRecord(extracted);
  if (buffer == NULL) {
    LCH_ListDestroy(extracted);
    return NULL;
  }
  LCH_ListDestroy(extracted);

  char *composed = LCH_BufferToString(buffer);
  return composed;
}

LCH_Dict *LCH_TableToDict(const LCH_List *const table,
                          const char *const primary,
                          const char *const subsidiary) {
  assert(primary != NULL);
  assert(table != NULL);

  LCH_List *primary_fields = LCH_CSVParseRecord(primary);
  if (primary_fields == NULL) {
    LCH_LOG_ERROR("Failed to parse primary fields '%s'.", primary);
    return NULL;
  }
  LCH_ListSort(primary_fields, (int (*)(const void *, const void *))strcmp);

  LCH_List *subsidiary_fields =
      (subsidiary == NULL) ? LCH_ListCreate() : LCH_CSVParseRecord(subsidiary);
  if (subsidiary_fields == NULL) {
    LCH_LOG_ERROR("Failed to parse primary fields '%s'.", subsidiary);
    LCH_ListDestroy(primary_fields);
    return NULL;
  }
  LCH_ListSort(subsidiary_fields, (int (*)(const void *, const void *))strcmp);

  LCH_Dict *dict = LCH_DictCreate();
  if (dict == NULL) {
    LCH_ListDestroy(subsidiary_fields);
    LCH_ListDestroy(primary_fields);
    return NULL;
  }

  const size_t num_records = LCH_ListLength(table);
  if (num_records < 1) {
    LCH_LOG_ERROR("Table is missing required header record.");
    LCH_DictDestroy(dict);
    LCH_ListDestroy(subsidiary_fields);
    LCH_ListDestroy(primary_fields);
    return NULL;
  }

  const LCH_List *const header = LCH_ListGet(table, 0);
  assert(header != NULL);

  const size_t header_len = LCH_ListLength(header);
  const size_t primary_len = LCH_ListLength(primary_fields);
  const size_t subsidiary_len = LCH_ListLength(subsidiary_fields);
  if (header_len != primary_len + subsidiary_len) {
    LCH_LOG_ERROR(
        "Number of primary- and subsidiary fields does not align with number "
        "of header fields (%zu + %zu != %zu).",
        primary_len, subsidiary_len, header_len);
    LCH_ListDestroy(subsidiary_fields);
    LCH_ListDestroy(primary_fields);
    LCH_DictDestroy(dict);
    return NULL;
  }

  if (num_records == 1) {
    LCH_ListDestroy(subsidiary_fields);
    LCH_ListDestroy(primary_fields);
    return dict;
  }

  LCH_List *primary_indices = GetIndexOfFields(header, primary_fields);
  if (primary_indices == NULL) {
    LCH_ListDestroy(subsidiary_fields);
    LCH_ListDestroy(primary_fields);
    LCH_DictDestroy(dict);
    return NULL;
  }

  LCH_List *subsidiary_indices = GetIndexOfFields(header, subsidiary_fields);
  if (subsidiary_indices == NULL) {
    LCH_ListDestroy(primary_indices);
    LCH_ListDestroy(subsidiary_fields);
    LCH_ListDestroy(primary_fields);
    LCH_DictDestroy(dict);
    return NULL;
  }

  for (size_t i = 1; i < num_records; i++) {
    const LCH_List *const record = LCH_ListGet(table, i);
    assert(record != NULL);

    const size_t record_len = LCH_ListLength(record);
    if (record_len != header_len) {
      LCH_LOG_ERROR(
          "Number of record fields does not align with number of header fields "
          "for record %zu (%zu != %zu).",
          i, record_len, header_len);
      LCH_ListDestroy(subsidiary_indices);
      LCH_ListDestroy(primary_indices);
      LCH_ListDestroy(subsidiary_fields);
      LCH_ListDestroy(primary_fields);
      LCH_DictDestroy(dict);
      return NULL;
    }

    char *const key = ComposeFieldsAtIndices(record, primary_indices);
    if (key == NULL) {
      LCH_LOG_ERROR("Failed to compose primary fields for record %zu.", i);
      LCH_ListDestroy(subsidiary_indices);
      LCH_ListDestroy(primary_indices);
      LCH_ListDestroy(subsidiary_fields);
      LCH_ListDestroy(primary_fields);
      LCH_DictDestroy(dict);
      return NULL;
    }

    char *value = NULL;
    if (LCH_ListLength(subsidiary_indices) > 0) {
      value = ComposeFieldsAtIndices(record, subsidiary_indices);
      if (value == NULL) {
        LCH_LOG_ERROR("Failed to compose subsidiary fields for record %zu.", i);
        free(key);
        LCH_ListDestroy(subsidiary_indices);
        LCH_ListDestroy(primary_indices);
        LCH_ListDestroy(subsidiary_fields);
        LCH_ListDestroy(primary_fields);
        LCH_DictDestroy(dict);
        return NULL;
      }
    }

    if (!LCH_DictSet(dict, key, value, free)) {
      free(value);
      free(key);
      LCH_ListDestroy(subsidiary_indices);
      LCH_ListDestroy(primary_indices);
      LCH_ListDestroy(subsidiary_fields);
      LCH_ListDestroy(primary_fields);
      LCH_DictDestroy(dict);
      return NULL;
    }
    free(key);
  }

  LCH_ListDestroy(subsidiary_indices);
  LCH_ListDestroy(primary_indices);
  LCH_ListDestroy(subsidiary_fields);
  LCH_ListDestroy(primary_fields);

  return dict;
}

/******************************************************************************/

static LCH_List *ParseConcatFields(const char *const primary,
                                   const char *const subsidiary,
                                   const bool sort) {
  assert(primary != NULL);

  LCH_List *primary_fields = LCH_CSVParseRecord(primary);
  if (primary_fields == NULL) {
    LCH_LOG_ERROR("Failed to parse primary fields '%s'.", primary);
    return NULL;
  }

  if (sort) {
    LCH_ListSort(primary_fields, (int (*)(const void *, const void *))strcmp);
  }

  LCH_List *subsidiary_fields =
      (subsidiary == NULL) ? LCH_ListCreate() : LCH_CSVParseRecord(subsidiary);
  if (subsidiary_fields == NULL) {
    LCH_LOG_ERROR("Failed to parse subsidiary fields '%s'.", subsidiary);
    LCH_ListDestroy(primary_fields);
    return NULL;
  }

  if (sort) {
    LCH_ListSort(subsidiary_fields,
                 (int (*)(const void *, const void *))strcmp);
  }

  LCH_List *record = LCH_ListMoveElements(primary_fields, subsidiary_fields);
  if (record == NULL) {
    LCH_ListDestroy(subsidiary_fields);
    LCH_ListDestroy(primary_fields);
    return NULL;
  }

  return record;
}

LCH_List *LCH_DictToTable(const LCH_Dict *const dict, const char *const primary,
                          const char *const subsidiary) {
  assert(dict != NULL);

  LCH_List *const header = ParseConcatFields(primary, subsidiary, true);
  if (header == NULL) {
    return NULL;
  }

  LCH_List *const table = LCH_ListCreate();
  if (table == NULL) {
    LCH_ListDestroy(header);
    return NULL;
  }

  if (!LCH_ListAppend(table, header, (void (*)(void *))LCH_ListDestroy)) {
    LCH_ListDestroy(header);
    LCH_ListDestroy(table);
    return NULL;
  }

  LCH_List *const keys = LCH_DictGetKeys(dict);
  if (keys == NULL) {
    LCH_ListDestroy(table);
    return NULL;
  }

  const size_t num_keys = LCH_ListLength(keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char *const key = LCH_ListGet(keys, i);
    char *const value = LCH_DictGet(dict, key);

    LCH_List *const record = ParseConcatFields(key, value, false);
    if (record == NULL) {
      LCH_ListDestroy(keys);
      LCH_ListDestroy(table);
      return NULL;
    }

    if (!LCH_ListAppend(table, record, (void (*)(void *))LCH_ListDestroy)) {
      LCH_ListDestroy(record);
      LCH_ListDestroy(keys);
      LCH_ListDestroy(table);
      return NULL;
    }
  }

  LCH_ListDestroy(keys);

  return table;
}

bool LCH_MarshalString(LCH_Buffer *const buffer, const char *const str) {
  assert(buffer != NULL);
  assert(str != NULL);

  size_t offset;
  if (!LCH_BufferAllocate(buffer, sizeof(uint32_t), &offset)) {
    return false;
  }

  const size_t before = LCH_BufferLength(buffer);
  if (!LCH_BufferPrintFormat(buffer, str)) {
    return false;
  }
  const size_t after = LCH_BufferLength(buffer);

  const uint32_t network_size = htonl(after - before);
  LCH_BufferSet(buffer, offset, &network_size, sizeof(uint32_t));

  return true;
}

const char *LCH_UnmarshalString(const char *buffer, char **const str) {
  assert(buffer != NULL);

  const uint32_t *const network_size = (uint32_t *)buffer;
  buffer += sizeof(uint32_t);
  const uint32_t size = ntohl(*network_size);

  *str = strndup(buffer, size);
  if (*str == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  buffer += size;
  return buffer;
}
