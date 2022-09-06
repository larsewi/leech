#include <assert.h>
#include <errno.h>
#include <string.h>

#include "debug_messenger.h"
#include "utils.h"

#define INITIAL_CAPACITY 8
#define LOAD_FACTOR 0.75f

typedef struct LCH_Buffer LCH_Buffer;

typedef struct LCH_Item {
  char *key;
  union {
    LCH_Buffer *buffer;
    char *string;
    long number;
    bool boolean;
  };
  LCH_Type type;
} LCH_Item;

struct LCH_Buffer {
  size_t length;
  size_t capacity;
  LCH_Item **buffer;
};

static LCH_Buffer *LCH_BufferCreate() {
  LCH_Buffer *buffer = (LCH_Buffer *)malloc(sizeof(LCH_Buffer));
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  buffer->length = 0;
  buffer->capacity = INITIAL_CAPACITY;
  buffer->buffer = (LCH_Item **)calloc(buffer->capacity, sizeof(LCH_Item *));

  if (buffer->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    free(buffer);
    return NULL;
  }

  LCH_LOG_DEBUG("Created buffer with capacity %d/%d", buffer->length, buffer->capacity);
  return buffer;
}

LCH_Array *LCH_ArrayCreate() {
  return LCH_BufferCreate();
}

LCH_Object *LCH_ObjectCreate() {
  return LCH_BufferCreate();
}

size_t LCH_ArrayLength(const LCH_Array *const array) {
  assert(array != NULL);
  return array->length;
}

size_t LCH_ObjectLength(const LCH_Object *const array) {
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
    array->buffer = (LCH_Item **)reallocarray(array->buffer, array->capacity * 2,
                                              sizeof(LCH_Item *));
    memset(array->buffer + array->capacity, 0, array->capacity);
    array->capacity *= 2;
    if (array->buffer == NULL) {
      LCH_LOG_ERROR("Failed to reallocate memory: %s", strerror(errno));
      return NULL;
    }
    LCH_LOG_DEBUG("Expanded buffer to capacity %d/%d", array->length, array->capacity);
  }

  // Create list item
  LCH_Item *item = (LCH_Item *)calloc(1, sizeof(LCH_Item));
  if (item == NULL) {
    return false;
  }

  switch (type) {
  case LCH_ARRAY:
    // fallthrough
  case LCH_OBJECT:
    item->buffer = (LCH_Buffer *)data;
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
    // fallthrough
  case LCH_OBJECT:
    *data = (void *)item->buffer;
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

static void LCH_BufferDestroy(LCH_Buffer *buffer) {
  if (buffer == NULL) {
    return;
  }
  assert(buffer->buffer != NULL);

  for (size_t i = 0; i < buffer->capacity; i++) {
    LCH_Item *item = buffer->buffer[i];
    if (item == NULL) {
      continue;
    }
    switch (item->type) {
    case LCH_OBJECT:
      free(item->key);
      //fallthrough
    case LCH_ARRAY:
      LCH_BufferDestroy(item->buffer);
      break;
    case LCH_STRING:
      free(item->string);
      break;
    default:
      break;
    }
    free(item);
  }
  free(buffer->buffer);
  free(buffer);
}


void LCH_ArrayDestroy(LCH_Array *array) {
  LCH_BufferDestroy(array);
}

void LCH_ObjectDestroy(LCH_Object *object) {
  LCH_BufferDestroy(object);
}

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
