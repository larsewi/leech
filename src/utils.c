#include <assert.h>
#include <errno.h>
#include <string.h>

#include "debug_messenger.h"
#include "utils.h"

#define INITIAL_CAPACITY 8
#define LOAD_FACTOR 0.75f

typedef struct LCH_Item {
  union {
    LCH_Array *array;
    LCH_Object *object;
    char *string;
    long number;
    bool boolean;
  };
  LCH_Type type;
} LCH_Item;

typedef struct LCH_Array {
  size_t length;
  size_t capacity;
  LCH_Item **buffer;
} LCH_Array;

typedef struct LCH_Object {
  /* data */
} LCH_Object;

LCH_Array *LCH_ArrayCreate() {
  LCH_Array *array = (LCH_Array *)malloc(sizeof(LCH_Array));
  if (array == NULL) {
    return NULL;
  }

  array->length = 0;
  array->capacity = INITIAL_CAPACITY;
  array->buffer = (LCH_Item **)malloc(array->capacity * sizeof(LCH_Item *));

  if (array->buffer == NULL) {
    free(array);
    return NULL;
  }
  return array;
}

size_t LCH_ArrayLength(const LCH_Array *const array) {
  assert(array != NULL);
  return array->length;
}

static bool ArrayAppend(LCH_Array *const array, const void *const data,
                        const LCH_Type type) {
  assert(array != NULL);
  assert(array->buffer != NULL);
  assert(array->capacity >= array->length);
  assert(data != NULL);

  // Increase buffer capacity if needed
  if (array->length >= array->capacity) {
    LCH_LOG_DEBUG(
        "LCH_Array exceeded current capacity: Reallocating array buffer");
    array->capacity *= 2;
    array->buffer = (LCH_Item **)reallocarray(array->buffer, array->capacity,
                                              sizeof(LCH_Item *));
    if (array->buffer == NULL) {
      LCH_LOG_ERROR("Failed to reallocate LCH_Array: %s", strerror(errno));
      return NULL;
    }
  }

  // Create list item
  LCH_Item *item = (LCH_Item *)malloc(sizeof(LCH_Item));
  if (item == NULL) {
    return false;
  }

  switch (type) {
  case LCH_ARRAY:
    item->array = (LCH_Array *)data;
    break;
  case LCH_OBJECT:
    item->object = (LCH_Object *)data;
    break;
  case LCH_STRING:
    item->string = (char *)data;
    break;
  case LCH_NUMBER:
    item->number = (*(long *)data);
    break;
  case LCH_BOOLEAN:
    item->boolean = (*(bool *)data);
    break;
  }

  item->type = type;
  array->buffer[array->length] = item;
  array->length += 1;

  return true;
}

bool LCH_ArrayAppendArray(LCH_Array *const array, const LCH_Array *const data) {
  assert(array != NULL);
  assert(data != NULL);
  return ArrayAppend(array, (void *)data, LCH_ARRAY);
}

bool LCH_ArrayAppendObject(LCH_Array *const array,
                           const LCH_Object *const data) {
  assert(array != NULL);
  assert(data != NULL);
  return ArrayAppend(array, (void *)data, LCH_OBJECT);
}

bool LCH_ArrayAppendString(LCH_Array *const array, const char *const data) {
  assert(array != NULL);
  assert(data != NULL);
  return ArrayAppend(array, (void *)data, LCH_STRING);
}

bool LCH_ArrayAppendNumber(LCH_Array *const array, const long data) {
  assert(array != NULL);
  return ArrayAppend(array, (void *)(&data), LCH_NUMBER);
}

bool LCH_ArrayAppendBoolean(LCH_Array *const array, const bool data) {
  assert(array != NULL);
  return ArrayAppend(array, (void *)(&data), LCH_BOOLEAN);
}

void ArrayGet(const LCH_Array *const array, const size_t index, void **data,
              const LCH_Type type) {
  assert(array != NULL);
  assert(array->buffer != NULL);
  assert(index < array->length);

  LCH_Item *item = array->buffer[index];
  assert(item != NULL);
  assert(item->type == type);

  switch (type) {
  case LCH_ARRAY:
    *data = (void *)item->array;
    break;
  case LCH_OBJECT:
    *data = (void *)item->object;
    break;
  case LCH_STRING:
    *data = (void *)item->string;
    break;
  case LCH_NUMBER:
    (*(long *)data) = item->number;
    break;
  case LCH_BOOLEAN:
    (*(bool *)data) = item->boolean;
    break;
  }
}

LCH_Array *LCH_ArrayGetArray(const LCH_Array *const array, const size_t index) {
  assert(array != NULL);
  LCH_Array *data = NULL;
  ArrayGet(array, index, (void **)(&data), LCH_ARRAY);
  return data;
}

LCH_Object *LCH_ArrayGetObject(const LCH_Array *const array,
                               const size_t index) {
  assert(array != NULL);
  LCH_Object *data = NULL;
  ArrayGet(array, index, (void **)(&data), LCH_OBJECT);
  return data;
}

char *LCH_ArrayGetString(const LCH_Array *const array, const size_t index) {
  assert(array != NULL);
  char *data = NULL;
  ArrayGet(array, index, (void **)(&data), LCH_STRING);
  return data;
}

long LCH_ArrayGetNumber(const LCH_Array *const array, const size_t index) {
  assert(array != NULL);
  long data;
  ArrayGet(array, index, (void **)(&data), LCH_NUMBER);
  return data;
}

bool LCH_ArrayGetBoolean(const LCH_Array *const array, const size_t index) {
  assert(array != NULL);
  bool data;
  ArrayGet(array, index, (void **)(&data), LCH_BOOLEAN);
  return data;
}

void LCH_ArrayDestroy(LCH_Array *array) {
  assert(array != NULL);
  assert(array->buffer != NULL);

  for (int i = 0; i < array->length; i++) {
    LCH_Item *item = array->buffer[i];
    assert(item != NULL);
    switch (item->type) {
    case LCH_ARRAY:
      LCH_ArrayDestroy(item->array);
      break;
    case LCH_OBJECT:
      LCH_ObjectDestroy(item->object);
      break;
    case LCH_STRING:
      free(item->string);
      break;
    default:
      break;
    }
  }
  free(array->buffer);
  free(array);
}

void LCH_ObjectDestroy(LCH_Object *object) {}

unsigned long LCH_Hash(char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++) != 0) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

LCH_Array *LCH_SplitString(const char *str, const char *del) {
  LCH_Array *list = LCH_ArrayCreate();
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
        LCH_ArrayDestroy(list);
        return NULL;
      }
      if (!ArrayAppend(list, (void *)s, LCH_STRING)) {
        LCH_ArrayDestroy(list);
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
      LCH_ArrayDestroy(list);
      return NULL;
    }
    if (!ArrayAppend(list, (void *)s, LCH_STRING)) {
      LCH_ArrayDestroy(list);
      return NULL;
    }
  }

  return list;
}
