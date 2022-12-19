#include <assert.h>
#include <errno.h>
#include <string.h>

#include "leech.h"

typedef struct LCH_Table {
  LCH_List *primaryFields;
  LCH_List *subsidiaryFields;
  const void *readLocator;
  LCH_List *(*readCallback)(const void *);
  const void *writeLocator;
  bool (*writeCallback)(const void *, const LCH_List *);
} LCH_Table;

LCH_Table *LCH_TableCreate(const LCH_TableCreateInfo *const createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->primaryFields != NULL);
  assert(createInfo->subsidiaryFields != NULL);
  assert(createInfo->readLocator != NULL);
  assert(createInfo->writeLocator != NULL);
  assert(createInfo->readCallback != NULL);
  assert(createInfo->writeCallback != NULL);

  LCH_Table *table = (LCH_Table *)calloc(1, sizeof(LCH_Table));
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  {
    LCH_List *tmp = LCH_ParseCSV(createInfo->primaryFields);
    if (tmp == NULL) {
      LCH_TableDestroy(table);
      return NULL;
    }
    assert(LCH_ListLength(tmp) == 1);
    table->primaryFields = LCH_ListGet(tmp, 0);
    LCH_ListSort(table->primaryFields, (int (*)(const void *, const void *))strcmp);
    LCH_ListDestroyShallow(tmp);
  }

  {
    LCH_List *tmp = LCH_ParseCSV(createInfo->subsidiaryFields);
    if (tmp == NULL) {
      LCH_TableDestroy(table);
      return NULL;
    }
    assert(LCH_ListLength(tmp) == 1);
    table->subsidiaryFields = LCH_ListGet(tmp, 0);
    LCH_ListDestroyShallow(tmp);
    LCH_ListSort(table->subsidiaryFields, (int (*)(const void *, const void *))strcmp);
  }

  table->readLocator = createInfo->readLocator;
  table->readCallback = createInfo->readCallback;
  table->writeLocator = createInfo->writeLocator;
  table->writeCallback = createInfo->writeCallback;

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
    const size_t *const index = (size_t *)LCH_ListGet(indices, i);

    char *field = (char *)LCH_ListGet(record, *index);
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

  LCH_Buffer *buf = LCH_ComposeCSV(tbl);
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
  LCH_List *const records = table->readCallback(table->readLocator);
  if (records == NULL) {
    return NULL;
  }

  LCH_Dict *data = LCH_DictCreate();
  if (data == NULL) {
    LCH_ListDestroy(records);
    return NULL;
  }

  const LCH_List *const header = LCH_ListGet(records, 0);

  LCH_List *primaryIndices = GetIndexOfFields(header, table->primaryFields);
  if (primaryIndices == NULL) {
    LCH_DictDestroy(data);
    LCH_ListDestroy(records);
    return NULL;
  }

  LCH_List *subsidiaryIndices = GetIndexOfFields(header, table->subsidiaryFields);
  if (subsidiaryIndices == NULL) {
    LCH_ListDestroy(primaryIndices);
    LCH_DictDestroy(data);
    LCH_ListDestroy(records);
    return NULL;
  }

  for (size_t i = 1; i < LCH_ListLength(records); i++) {
    const LCH_List *const record = (LCH_List *)LCH_ListGet(records, i);

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

void LCH_TableDestroy(LCH_Table *table) {
  if (table == NULL) {
    return;
  }
  LCH_ListDestroy(table->subsidiaryFields);
  LCH_ListDestroy(table->primaryFields);
  free(table);
}
