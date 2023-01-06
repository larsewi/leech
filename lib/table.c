#include "table.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "csv.h"
#include "definitions.h"
#include "leech.h"
#include "list.h"
#include "utils.h"

typedef struct LCH_Table {
  const char *identifier;
  LCH_List *primary_fields;
  LCH_List *subsidiary_fields;
  const void *read_locator;
  LCH_List *(*read_callback)(const void *);
  const void *write_locator;
  bool (*write_callback)(const void *, const LCH_List *);
} LCH_Table;

const char *LCH_TableGetIdentifier(const LCH_Table *const self) {
  assert(self != NULL);
  return self->identifier;
}

LCH_Table *LCH_TableCreate(const LCH_TableCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->identifier != NULL);
  assert(createInfo->primary_fields != NULL);
  assert(createInfo->subsidiary_fields != NULL);
  assert(createInfo->read_locator != NULL);
  assert(createInfo->write_locator != NULL);
  assert(createInfo->read_callback != NULL);
  assert(createInfo->write_callback != NULL);

  LCH_Table *table = (LCH_Table *)calloc(1, sizeof(LCH_Table));
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  {
    LCH_List *tmp = LCH_CSVParse(createInfo->primary_fields);
    if (tmp == NULL) {
      LCH_TableDestroy(table);
      return NULL;
    }
    assert(LCH_ListLength(tmp) == 1);
    table->primary_fields = LCH_ListGet(tmp, 0);
    LCH_ListSort(table->primary_fields,
                 (int (*)(const void *, const void *))strcmp);
    LCH_ListDestroyShallow(tmp);
  }

  {
    LCH_List *tmp = LCH_CSVParse(createInfo->subsidiary_fields);
    if (tmp == NULL) {
      LCH_TableDestroy(table);
      return NULL;
    }
    assert(LCH_ListLength(tmp) == 1);
    table->subsidiary_fields = LCH_ListGet(tmp, 0);
    LCH_ListDestroyShallow(tmp);
    LCH_ListSort(table->subsidiary_fields,
                 (int (*)(const void *, const void *))strcmp);
  }

  table->identifier = createInfo->identifier;
  table->read_locator = createInfo->read_locator;
  table->read_callback = createInfo->read_callback;
  table->write_locator = createInfo->write_locator;
  table->write_callback = createInfo->write_callback;

  return table;
}

static LCH_List *GetIndexOfFields(const LCH_List *const header,
                                  const LCH_List *const fields) {
  const size_t n_cols = LCH_ListLength(header);

  LCH_List *indices = LCH_ListCreate();
  if (indices == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < LCH_ListLength(fields); i++) {
    void *const field = LCH_ListGet(fields, i);

    size_t *const index = malloc(sizeof(size_t));
    if (index == NULL) {
      LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
      LCH_ListDestroy(indices);
      return NULL;
    }
    *index = LCH_ListIndex(header, field,
                           (int (*)(const void *, const void *))strcmp);

    if (*index >= n_cols) {
      LCH_LOG_ERROR("Field '%s' not in table header", (char *)field);
      free(index);
      LCH_ListDestroy(indices);
      return NULL;
    }

    if (!LCH_ListAppend(indices, index, free)) {
      free(index);
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

  for (size_t i = 0; i < LCH_ListLength(indices); i++) {
    const size_t index = *(size_t *)LCH_ListGet(indices, i);
    assert(index < LCH_ListLength(record));

    char *field = (char *)LCH_ListGet(record, index);
    field = strdup(field);
    if (field == NULL) {
      LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
      LCH_ListDestroy(fields);
      return NULL;
    }

    if (!LCH_ListAppend(fields, field, free)) {
      return NULL;
    }
  }

  return fields;
}

static char *ComposeFieldsAtIndices(const LCH_List *const record,
                                    const LCH_List *const indices) {
  LCH_List *const lst = ExtractFieldsAtIndices(record, indices);
  if (lst == NULL) {
    return NULL;
  }

  LCH_List *tbl = LCH_ListCreate();
  if (tbl == NULL) {
    LCH_ListDestroy(lst);
  }

  if (!LCH_ListAppend(tbl, lst, (void (*)(void *))LCH_ListDestroy)) {
    LCH_ListDestroy(tbl);
    LCH_ListDestroy(lst);
  }

  LCH_Buffer *buf = LCH_CSVCompose(tbl);
  LCH_ListDestroy(tbl);
  if (buf == NULL) {
    return NULL;
  }

  char *str = LCH_BufferGet(buf);
  LCH_BufferDestroy(buf);
  if (str == NULL) {
    return NULL;
  }

  return str;
}

LCH_Dict *LCH_TableLoadNewData(const LCH_Table *const table) {
  LCH_List *const records = table->read_callback(table->read_locator);
  if (records == NULL) {
    return NULL;
  }

  LCH_Dict *data = LCH_DictCreate();
  if (data == NULL) {
    LCH_ListDestroy(records);
    return NULL;
  }

  const LCH_List *const header = LCH_ListGet(records, 0);
  const size_t header_len = LCH_ListLength(header);

  LCH_List *primaryIndices = GetIndexOfFields(header, table->primary_fields);
  if (primaryIndices == NULL) {
    LCH_DictDestroy(data);
    LCH_ListDestroy(records);
    return NULL;
  }

  LCH_List *subsidiaryIndices =
      GetIndexOfFields(header, table->subsidiary_fields);
  if (subsidiaryIndices == NULL) {
    LCH_ListDestroy(primaryIndices);
    LCH_DictDestroy(data);
    LCH_ListDestroy(records);
    return NULL;
  }

  for (size_t i = 1; i < LCH_ListLength(records); i++) {
    const LCH_List *const record = (LCH_List *)LCH_ListGet(records, i);
    const size_t record_len = LCH_ListLength(record);
    if (record_len != header_len) {
      LCH_LOG_ERROR(
          "Number of header columns does not align with number of columns in "
          "row %zu (%zu != %zu)",
          i, header_len, record_len);
      LCH_ListDestroy(subsidiaryIndices);
      LCH_ListDestroy(primaryIndices);
      LCH_DictDestroy(data);
      LCH_ListDestroy(records);
      return NULL;
    }

    char *key = ComposeFieldsAtIndices(record, primaryIndices);
    if (key == NULL) {
      LCH_ListDestroy(subsidiaryIndices);
      LCH_ListDestroy(primaryIndices);
      LCH_DictDestroy(data);
      LCH_ListDestroy(records);
      return NULL;
    }

    char *value = ComposeFieldsAtIndices(record, subsidiaryIndices);
    if (value == NULL) {
      free(key);
      LCH_ListDestroy(subsidiaryIndices);
      LCH_ListDestroy(primaryIndices);
      LCH_DictDestroy(data);
      LCH_ListDestroy(records);
      return NULL;
    }

    if (!LCH_DictSet(data, key, value, free)) {
      free(value);
      free(key);
      LCH_ListDestroy(subsidiaryIndices);
      LCH_ListDestroy(primaryIndices);
      LCH_DictDestroy(data);
      LCH_ListDestroy(records);
      return NULL;
    }
    free(key);
  }

  LCH_ListDestroy(subsidiaryIndices);
  LCH_ListDestroy(primaryIndices);
  LCH_ListDestroy(records);
  return data;
}

static LCH_Dict *LoadSnapshot(const LCH_Table *const self,
                              const char *const path) {
  assert(self != NULL);

  LCH_List *table = LCH_CSVParseFile(path);
  if (table == NULL) {
    return NULL;
  }

  LCH_Dict *snapshot = LCH_DictCreate();
  if (snapshot == NULL) {
    return NULL;
  }

  assert(LCH_ListLength(table) % 2 == 0);
  char *key = NULL, *val = NULL;
  for (size_t i = 0; i < LCH_ListLength(table); i++) {
    const LCH_List *const record = LCH_ListGet(table, i);

    LCH_Buffer *const buffer = LCH_CSVComposeRecord(record);
    if (buffer == NULL) {
      LCH_ListDestroy(table);
      LCH_DictDestroy(snapshot);
      return NULL;
    }

    char *str = LCH_BufferGet(buffer);
    if (str == NULL) {
      LCH_LOG_ERROR("Failed to get snapshot string from string buffer");
      free(key);
      free(val);
      LCH_BufferDestroy(buffer);
      LCH_ListDestroy(table);
      LCH_DictDestroy(snapshot);
      return NULL;
    }
    LCH_BufferDestroy(buffer);

    if (i % 2 == 0) {
      key = str;
      continue;
    }
    val = str;

    if (!LCH_DictSet(snapshot, key, val, free)) {
      free(key);
      free(val);
      LCH_ListDestroy(table);
      LCH_DictDestroy(snapshot);
      return NULL;
    }
    free(key);
    key = NULL;
    val = NULL;
  }

  LCH_ListDestroy(table);
  return snapshot;
}

LCH_Dict *LCH_TableLoadOldData(const LCH_Table *const table,
                               const char *const work_dir) {
  assert(table != NULL);
  assert(work_dir != NULL);

  assert(table->identifier != NULL);
  const char *const id = table->identifier;

  char path[PATH_MAX];
  int ret = snprintf(path, sizeof(path), "%s%c%s%c%s", work_dir, PATH_SEP,
                     "snapshot", PATH_SEP, id);
  if ((ret < 0) || ((size_t)ret >= sizeof(path))) {
    LCH_LOG_ERROR("Failed to join paths: Truncation error");
    return NULL;
  }

  LCH_Dict *const snapshot =
      (LCH_IsRegularFile(path)) ? LoadSnapshot(table, path) : LCH_DictCreate();
  if (snapshot == NULL) {
    LCH_LOG_ERROR("Failed to load snapshot for table %s", table->identifier);
    return NULL;
  }

  return snapshot;
}

void LCH_TableDestroy(LCH_Table *table) {
  if (table == NULL) {
    return;
  }
  LCH_ListDestroy(table->subsidiary_fields);
  LCH_ListDestroy(table->primary_fields);
  free(table);
}
