#include "utils.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "buffer.h"
#include "csv.h"
#include "definitions.h"
#include "list.h"
#include "sha1.h"

char *LCH_StringDuplicate(const char *const str) {
  if (str == NULL) {
    return NULL;
  }

  char *const dup = strdup(str);
  if (dup == NULL) {
    LCH_LOG_ERROR("strdup(3): Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }
  return dup;
}

void *LCH_Allocate(const size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memeory: %s", strerror(errno));
    return NULL;
  }
  ptr = memset(ptr, 0, size);
  return ptr;
}

bool LCH_StringEqual(const char *const str1, const char *const str2) {
  assert(str1 != NULL);
  assert(str2 != NULL);
  return strcmp(str1, str2) == 0;
}

/******************************************************************************/

LCH_List *LCH_StringSplit(const char *str, const char *del) {
  assert(str != NULL);
  assert(del != NULL);

  LCH_List *const list = LCH_ListCreate();

  const char *start = str;
  const char *end = strpbrk(str, del);

  while (end != NULL) {
    char *tmp = strndup(start, end - start);
    if (tmp == NULL) {
      LCH_LOG_ERROR("strndup(): Failed to allocate memory: %s",
                    strerror(errno));
      return NULL;
    }
    LCH_ListAppend(list, tmp, free);
    start = end + 1;
    end = strpbrk(start, del);
  }

  char *tmp = LCH_StringDuplicate(start);
  if (tmp == NULL) {
    return NULL;
  }
  LCH_ListAppend(list, tmp, free);
  return list;
}

/******************************************************************************/

char *LCH_StringJoin(const LCH_List *const list, const char *const del) {
  LCH_Buffer *const buffer = LCH_BufferCreate();

  const size_t len = LCH_ListLength(list);
  for (size_t i = 0; i < len; i++) {
    if (i > 0) {
      if (!LCH_BufferPrintFormat(buffer, "%s", del)) {
        LCH_BufferDestroy(buffer);
        return NULL;
      }
    }

    const char *const str = (const char *)LCH_ListGet(list, i);
    if (!LCH_BufferPrintFormat(buffer, "%s", str)) {
      LCH_BufferDestroy(buffer);
      return NULL;
    }
  }

  char *const result = LCH_BufferToString(buffer);
  return result;
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

  str = (char *)memmove(str, (str + start), end - start);
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
  struct stat sb;
  memset(&sb, 0, sizeof(struct stat));
  return stat(path, &sb) == 0;
}

/******************************************************************************/

bool LCH_IsRegularFile(const char *const path) {
  struct stat sb;
  memset(&sb, 0, sizeof(struct stat));
  return (stat(path, &sb) == 0) && ((sb.st_mode & S_IFMT) == S_IFREG);
}

/******************************************************************************/

bool LCH_IsDirectory(const char *const path) {
  struct stat sb;
  memset(&sb, 0, sizeof(struct stat));
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
      path[used++] = LCH_PATH_SEP;
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

char *LCH_FileRead(const char *const path, size_t *const length) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return NULL;
  }

  if (!LCH_BufferReadFile(buffer, path)) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  if (length != NULL) {
    *length = LCH_BufferLength(buffer);
  }

  char *str = LCH_BufferToString(buffer);
  assert(str != NULL);
  return str;
}

/******************************************************************************/

bool LCH_FileWrite(const char *const path, const char *const str) {
  if (!LCH_CreateParentDirectories(path)) {
    LCH_LOG_ERROR("Failed to create parent directories for file '%s'", path);
    return false;
  }

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

bool LCH_FileDelete(const char *const filename) {
  if (unlink(filename) != 0) {
    LCH_LOG_ERROR("Failed to delete file '%s': %s", filename, strerror(errno));
    return false;
  }
  return true;
}

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

bool LCH_ParseNumber(const char *const str, long *const number) {
  assert(str != NULL);
  assert(number != NULL);

  char *endptr;
  errno = 0;  // To distinguish success/failure after call
  const long value = strtol(str, &endptr, 10);

  if (errno != 0) {
    LCH_LOG_ERROR("Failed to parse number '%s': %s", str, strerror(errno));
    return false;
  }

  if (endptr == str) {
    LCH_LOG_ERROR("Failed to parse number '%s': No digits were found", str);
    return false;
  }

  if (*endptr != '\0') {
    LCH_LOG_WARNING(
        "Found trailing characters '%s' after parsing number '%ld' from string "
        "'%s'",
        endptr, value, str);
  }

  *number = value;
  return true;
}

bool LCH_ParseVersion(const char *const str, size_t *const v_major,
                      size_t *const v_minor, size_t *const v_patch) {
  assert(str != NULL);
  assert(v_major != NULL);
  assert(v_minor != NULL);
  assert(v_patch != NULL);

  LCH_List *const list = LCH_StringSplit(str, ".");
  const size_t length = LCH_ListLength(list);

  static const char *const error_messages[] = {
      "Missing major version number",
      "Missing minor version number",
      "Missing patch version number",
      "Too many version numbers",
  };
  if (length < 3 || length > 3) {
    LCH_LOG_ERROR("Failed to parse version '%s': %s",
                  error_messages[LCH_MIN(length, 3)]);
    LCH_ListDestroy(list);
    return false;
  }

  long val;
  const char *sub = (char *)LCH_ListGet(list, 0);
  if (!LCH_ParseNumber(sub, &val)) {
    LCH_ListDestroy(list);
    return false;
  }
  if (val < 0) {
    LCH_LOG_ERROR("Failed to parse version '%s': Bad major version number %ld",
                  str, val);
    LCH_ListDestroy(list);
    return false;
  }
  *v_major = (size_t)val;

  sub = (char *)LCH_ListGet(list, 1);
  if (!LCH_ParseNumber(sub, &val)) {
    LCH_ListDestroy(list);
    return false;
  }
  if (val < 0) {
    LCH_LOG_ERROR("Failed to parse version '%s': Bad major version number %ld",
                  str, val);
    LCH_ListDestroy(list);
    return false;
  }
  *v_minor = (size_t)val;

  sub = (char *)LCH_ListGet(list, 2);
  if (!LCH_ParseNumber(sub, &val)) {
    LCH_ListDestroy(list);
    return false;
  }
  if (val < 0) {
    LCH_LOG_ERROR("Failed to parse version '%s': Bad major version number %ld",
                  str, val);
    LCH_ListDestroy(list);
    return false;
  }

  LCH_ListDestroy(list);
  *v_patch = (size_t)val;
  return true;
}

char *LCH_StringFormat(const char *const format, ...) {
  assert(format != NULL);

  va_list ap;
  va_start(ap, format);
  const int length = vsnprintf(NULL, 0, format, ap);
  assert(length >= 0);
  va_end(ap);

  char *const str = (char *)malloc((size_t)length + 1);
  if (str == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  va_start(ap, format);
  const int ret = vsnprintf(str, (size_t)length + 1, format, ap);
  LCH_UNUSED(ret);
  va_end(ap);
  assert(ret == length);

  return str;
}

bool LCH_CreateParentDirectories(const char *const filename) {
  assert(filename != NULL);

  char fcopy[strlen(filename) + 1];
  strcpy(fcopy, filename);
  char *parent = dirname(fcopy);

  LCH_List *const dirs = LCH_ListCreate();
  struct stat sb;

  while (stat(parent, &sb) == -1) {
    char *const dir = LCH_StringDuplicate(parent);
    if (dir == NULL) {
      LCH_ListDestroy(dirs);
      return false;
    }

    if (!LCH_ListAppend(dirs, dir, free)) {
      free(dir);
      LCH_ListDestroy(dirs);
      return false;
    }

    parent = dirname(parent);
  }

  const size_t num_dirs = LCH_ListLength(dirs);
  for (size_t i = num_dirs; i > 0; i--) {
    char *const dir = (char *)LCH_ListGet(dirs, i - 1);
    if (mkdir(dir, (mode_t)0700) == -1) {
      LCH_LOG_ERROR("Failed to create parent directory '%s' for file '%s': %s",
                    dir, filename, strerror(errno));
      LCH_ListDestroy(dirs);
      return false;
    }
    LCH_LOG_VERBOSE("Created directory '%s' with mode %o", dir, (mode_t)0700);
  }
  LCH_ListDestroy(dirs);
  return true;
}

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

bool LCH_ListAppendBufferDuplicate(LCH_List *const list,
                                   const LCH_Buffer *const buffer) {
  LCH_Buffer *const duplicate = LCH_BufferDuplicate(buffer);
  if (duplicate == NULL) {
    return false;
  }

  return LCH_ListAppend(list, duplicate, LCH_BufferDestroy);
}

char *LCH_StringTruncate(const char *const str, const size_t len,
                         const size_t max) {
  assert(max >= 3);  // We need at least Bytes for ...

  LCH_Buffer *const buffer = LCH_BufferCreate();
  for (size_t i = 0; i < max; i++) {
    if ((i < len) && (str[i] == '\0')) {
      return LCH_BufferToString(buffer);
    }
    if (!LCH_BufferAppend(buffer, str[i])) {
      LCH_BufferDestroy(buffer);
      return NULL;
    }
  }

  LCH_BufferChop(buffer, max - 3);
  if (!LCH_BufferPrintFormat(buffer, "...")) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  return LCH_BufferToString(buffer);
}

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
