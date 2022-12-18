#include <assert.h>
#include <errno.h>
#include <string.h>

#include "leech.h"

typedef struct LCH_Table {
  LCH_List *primaryFields;
  LCH_List *subsidiaryFields;
  LCH_Dict *data;
  const void *writeLocator;
  bool (*writeCallback)(const void *, const LCH_List *);
} LCH_Table;

static LCH_List *GetIndexOfFields(const LCH_List *const header, const LCH_List *const fields) {
  const size_t n_cols = LCH_ListLength(header);

  LCH_List *indices = LCH_ListCreate();
  for (size_t i = 0; i < LCH_ListLength(fields); i++) {
    void *const field = LCH_ListGet(fields, i);

    size_t *const index = malloc(sizeof(size_t));
    if (index == NULL) {
      LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
      LCH_ListDestroy(indices);
      return NULL;
    }
    *index = LCH_ListIndex(header, field, (int (*)(const void *, const void *))strcmp);

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

static LCH_List *ExtractFieldsAtIndices(const LCH_List *const record, const LCH_List *const indices) {
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

static char *ComposeFieldsAtIndices(const LCH_List *const record, const LCH_List *const indices) {
  LCH_List *const lst = ExtractFieldsAtIndices(record, indices);
  if (lst == NULL) {
    return NULL;
  }

  LCH_Buffer *buf = LCH_ComposeCSV(lst);
  LCH_ListDestroy(lst);
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

LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo) {
  assert(createInfo != NULL);
  assert(createInfo->primaryFields != NULL);
  assert(createInfo->subsidiaryFields != NULL);
  assert(createInfo->readLocator != NULL);
  assert(createInfo->writeLocator != NULL);
  assert(createInfo->readCallback != NULL);
  assert(createInfo->writeCallback != NULL);

  LCH_Table *table = (LCH_Table *)calloc(1, sizeof(LCH_Table));
  if (table == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for table: %s", strerror(errno));
    return NULL;
  }

  LCH_List *primaryFields = LCH_ParseCSV(createInfo->primaryFields);
  LCH_ListSort(primaryFields, (int (*)(const void *, const void *))strcmp);
  table->primaryFields = primaryFields;

  LCH_List *subsidiaryFields = LCH_ParseCSV(createInfo->subsidiaryFields);
  LCH_ListSort(subsidiaryFields, (int (*)(const void *, const void *))strcmp);
  table->subsidiaryFields = subsidiaryFields;

  LCH_List *const records = createInfo->readCallback(createInfo->readLocator);
  if (records == NULL) {
    LCH_ListDestroy(subsidiaryFields);
    LCH_ListDestroy(primaryFields);
    free(table);
    return NULL;
  }

  const LCH_List *const header = LCH_ListGet(records, 0);

  LCH_List *primaryIndices = GetIndexOfFields(header, primaryFields);
  if (primaryFields == NULL) {
    LCH_ListDestroy(subsidiaryFields);
    LCH_ListDestroy(primaryFields);
    free(table);
    return NULL;
  }

  LCH_List *subsidiaryIndices = GetIndexOfFields(header, subsidiaryFields);
  if (primaryFields == NULL) {
    LCH_ListDestroy(primaryIndices);
    LCH_ListDestroy(subsidiaryFields);
    LCH_ListDestroy(primaryFields);
    free(table);
    return NULL;
  }

  LCH_Dict *data = LCH_DictCreate();
  if (data == NULL) {
    LCH_ListDestroy(subsidiaryIndices);
    LCH_ListDestroy(primaryIndices);
    LCH_ListDestroy(subsidiaryFields);
    LCH_ListDestroy(primaryFields);
    free(table);
    return NULL;
  }

  for (size_t i = 1; i < LCH_ListLength(records); i++) {
    const LCH_List *const record = (LCH_List *)LCH_ListGet(records, i);

    char *key = ComposeFieldsAtIndices(record, primaryIndices);
    if (key == NULL) {
      LCH_DictDestroy(data);
      LCH_ListDestroy(subsidiaryIndices);
      LCH_ListDestroy(primaryIndices);
      LCH_ListDestroy(subsidiaryFields);
      LCH_ListDestroy(primaryFields);
      free(table);
      return NULL;
    }

    char *value = ComposeFieldsAtIndices(record, subsidiaryIndices);
    if (value == NULL) {
      free(key);
      LCH_DictDestroy(data);
      LCH_ListDestroy(subsidiaryIndices);
      LCH_ListDestroy(primaryIndices);
      LCH_ListDestroy(subsidiaryFields);
      LCH_ListDestroy(primaryFields);
      free(table);
      return NULL;
    }

    if(!LCH_DictSet(data, key, value, free)) {
      free(value);
      free(key);
      LCH_DictDestroy(data);
      LCH_ListDestroy(subsidiaryIndices);
      LCH_ListDestroy(primaryIndices);
      LCH_ListDestroy(subsidiaryFields);
      LCH_ListDestroy(primaryFields);
      free(table);
      return NULL;
    }
  }

  LCH_ListDestroy(subsidiaryIndices);
  LCH_ListDestroy(primaryIndices);
  LCH_ListDestroy(records);

  table->data = data;
  table->writeLocator = createInfo->writeLocator;
  table->writeCallback = createInfo->writeCallback;

  return table;
}

void LCH_TableDestroy(LCH_Table *table) {
  if (table == NULL) {
    return;
  }
  LCH_DictDestroy(table->data);
  LCH_ListDestroy(table->subsidiaryFields);
  LCH_ListDestroy(table->primaryFields);
  free(table);
}
