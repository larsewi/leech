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
  void *value;
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

LCH_List *LCH_ListCreate() {
  return LCH_BufferCreate();
}

LCH_Dict *LCH_DictCreate() {
  return LCH_BufferCreate();
}

size_t LCH_ListLength(const LCH_List *const list) {
  assert(list != NULL);
  return list->length;
}

size_t LCH_DictLength(const LCH_Dict *const dict) {
  assert(dict != NULL);
  return dict->length;
}

bool LCH_ListAppend(LCH_List *const list, void *const data) {
  assert(list != NULL);
  assert(list->buffer != NULL);
  assert(list->capacity >= list->length);
  assert(data != NULL);

  // Increase buffer capacity if needed
  if (list->length == list->capacity) {
    list->buffer = (LCH_Item **)reallocarray(list->buffer, list->capacity * 2,
                                              sizeof(LCH_Item *));
    memset(list->buffer + list->capacity, 0, list->capacity);
    list->capacity *= 2;
    if (list->buffer == NULL) {
      LCH_LOG_ERROR("Failed to reallocate memory: %s", strerror(errno));
      return NULL;
    }
    LCH_LOG_DEBUG("Expanded buffer to capacity %d/%d", list->length, list->capacity);
  }

  // Create item
  LCH_Item *item = (LCH_Item *)calloc(1, sizeof(LCH_Item));
  if (item == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return false;
  }
  item->value = data;

  // Insert item into buffer
  list->buffer[list->length] = item;
  list->length += 1;

  return true;
}


void *LCH_ListGet(const LCH_List *const list, const size_t index) {
  assert(list != NULL);
  assert(list->buffer != NULL);
  assert(index < list->length);

  LCH_Item *item = list->buffer[index];
  return item->value;
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
    free(item->key);
    free(item->value);
    free(item);
  }
  free(buffer->buffer);
  free(buffer);
}


void LCH_ListDestroy(LCH_List *list) {
  LCH_BufferDestroy(list);
}

void LCH_DictDestroy(LCH_Dict *dict) {
  LCH_BufferDestroy(dict);
}

unsigned long LCH_Hash(char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++) != 0) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

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
        LCH_ListDestroy(list);
        return NULL;
      }
      if (!LCH_ListAppend(list, (void *)s)) {
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
    if (!LCH_ListAppend(list, (void *)s)) {
      LCH_ListDestroy(list);
      return NULL;
    }
  }

  return list;
}
