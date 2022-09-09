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
  void (*destroy)(void *);
} LCH_Item;

struct LCH_Buffer {
  size_t length;
  size_t capacity;
  LCH_Item **buffer;
};

static LCH_Buffer *LCH_BufferCreate(void) {
  LCH_Buffer *self = (LCH_Buffer *)malloc(sizeof(LCH_Buffer));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  self->length = 0;
  self->capacity = INITIAL_CAPACITY;
  self->buffer = (LCH_Item **)calloc(self->capacity, sizeof(LCH_Item *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    free(self);
    return NULL;
  }

  LCH_LOG_DEBUG("Created buffer with capacity %d/%d", self->length,
                self->capacity);
  return self;
}

LCH_List *LCH_ListCreate(void) { return LCH_BufferCreate(); }

LCH_Dict *LCH_DictCreate(void) { return LCH_BufferCreate(); }

static size_t LCH_BufferLength(const LCH_Buffer *const self) {
  assert(self != NULL);
  return self->length;
}

size_t LCH_ListLength(const LCH_List *const self) {
  return LCH_BufferLength(self);
}

size_t LCH_DictLength(const LCH_Dict *const self) {
  return LCH_BufferLength(self);
}

static bool ListCapacity(LCH_List *const self) {
  if (self->length < self->capacity) {
    return true;
  }
  self->buffer = (LCH_Item **)reallocarray(self->buffer, self->capacity * 2,
                                           sizeof(LCH_Item *));
  memset(self->buffer + self->capacity, 0, self->capacity);
  self->capacity *= 2;
  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to reallocate memory: %s", strerror(errno));
    return false;
  }
  LCH_LOG_DEBUG("Expanded list capacity %d/%d", self->length, self->capacity);
  return true;
}

bool LCH_ListAppend(LCH_List *const self, void *const value,
                    void (*destroy)(void *)) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(self->capacity >= self->length);
  assert(value != NULL);

  if (!ListCapacity(self)) {
    return false;
  }

  // Create item
  LCH_Item *item = (LCH_Item *)calloc(1, sizeof(LCH_Item));
  if (item == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return false;
  }
  item->value = value;
  item->destroy = destroy;

  // Insert item into buffer
  self->buffer[self->length] = item;
  self->length += 1;

  return true;
}

static size_t Hash(const char *const str) {
  size_t hash = 5381, len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    hash = ((hash << 5) + hash) + str[i];
  }
  return hash;
}

static bool DictCapacity(LCH_Dict *const self) {
  if (self->length < self->capacity / LOAD_FACTOR) {
    return true;
  }

  size_t new_capacity = self->capacity * 2;
  LCH_Item **new_buffer = (LCH_Item **)calloc(new_capacity, sizeof(LCH_Item *));
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return false;
  }

  for (size_t i = 0; i < self->capacity; i++) {
    if (self->buffer[i] == NULL) {
      continue;
    }
    LCH_Item *item = self->buffer[i];

    long index = Hash(item->key) % new_capacity;
    while (new_buffer[index] != NULL) {
      index += 1;
    }
    new_buffer[index] = item;
  }

  self->buffer = new_buffer;
  self->capacity = new_capacity;
  LCH_LOG_DEBUG("Expanded dict capacity %d/%d", self->length, self->capacity);

  return true;
}

bool LCH_DictSet(LCH_Dict *const self, const char *const key, void *const value,
                 void (*destroy)(void *)) {
  assert(self != NULL);
  assert(key != NULL);

  if (!DictCapacity(self)) {
    return false;
  }

  long index = Hash(key) % self->capacity;

  while (self->buffer[index] != NULL &&
         strcmp(self->buffer[index]->key, key) != 0) {
    index++;
  }

  if (strcmp(self->buffer[index]->key, key) == 0) {
    LCH_Item *item = self->buffer[index];
    item->destroy(item->value);
    item->value = value;
    item->destroy = destroy;
    return true;
  }

  LCH_Item *item = (LCH_Item *)calloc(1, sizeof(LCH_Item));
  if (item == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return false;
  }

  item->key = strdup(key);
  if (item->key == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    free(item);
    return false;
  }
  item->value = value;
  item->destroy = destroy;

  self->buffer[index] = item;

  return true;
}

void *LCH_ListGet(const LCH_List *const self, const size_t index) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(index < self->length);

  LCH_Item *item = self->buffer[index];
  return item->value;
}

bool LCH_DictHasKey(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  long index = Hash(key) % self->capacity;
  while (self->buffer[index] != NULL ||
         strcmp(self->buffer[index]->key, key) == 0) {
    index += 1;
  }
  return strcmp(self->buffer[index]->key, key) == 0;
}

void *LCH_DictGet(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  long index = Hash(key) % self->capacity;
  while (self->buffer[index] != NULL ||
         strcmp(self->buffer[index]->key, key) == 0) {
    index += 1;
  }
  assert(self->buffer[index] != NULL);

  return self->buffer[index]->value;
}

static void LCH_BufferDestroy(LCH_Buffer *self) {
  if (self == NULL) {
    return;
  }
  assert(self->buffer != NULL);

  for (size_t i = 0; i < self->capacity; i++) {
    LCH_Item *item = self->buffer[i];
    if (item == NULL) {
      continue;
    }
    free(item->key);
    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
    free(item);
  }
  free(self->buffer);
  free(self);
}

void LCH_ListDestroy(LCH_List *self) { LCH_BufferDestroy(self); }

void LCH_DictDestroy(LCH_Dict *self) { LCH_BufferDestroy(self); }

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
      if (!LCH_ListAppend(list, (void *)s, free)) {
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
