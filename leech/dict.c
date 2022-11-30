#include <assert.h>
#include <errno.h>
#include <string.h>

#include "debug_messenger.h"
#include "definitions.h"
#include "dict.h"

#define INITIAL_CAPACITY 8
#define LOAD_FACTOR 0.75f

typedef struct LCH_DictElement {
  char *key;
  void *value;
  void (*destroy)(void *);
} LCH_DictElement;

struct LCH_Dict {
  size_t length;
  size_t capacity;
  LCH_DictElement **buffer;
};

LCH_Dict *LCH_DictCreate() {
  LCH_Dict *self = (LCH_Dict *)malloc(sizeof(LCH_Dict));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict: %s", strerror(errno));
    return NULL;
  }

  self->length = 0;
  self->capacity = INITIAL_CAPACITY;
  self->buffer =
      (LCH_DictElement **)calloc(self->capacity, sizeof(LCH_DictElement *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict buffer: %s",
                  strerror(errno));
    free(self);
    return NULL;
  }

  LCH_LOG_DEBUG("Created dict buffer with capacity %d/%d", self->length,
                self->capacity);
  return self;
}

size_t LCH_DictLength(const LCH_Dict *const self) {
  assert(self != NULL);
  return self->length;
}

static size_t Hash(const char *const str) {
  size_t hash = 5381, len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    hash = ((hash << 5) + hash) + str[i];
  }
  return hash;
}

static bool DictCapacity(LCH_Dict *const self) {
  if (self->length < self->capacity * LOAD_FACTOR) {
    return true;
  }

  size_t new_capacity = self->capacity * 2;
  LCH_DictElement **new_buffer =
      (LCH_DictElement **)calloc(new_capacity, sizeof(LCH_DictElement *));
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict element: %s",
                  strerror(errno));
    return false;
  }

  for (size_t i = 0; i < self->capacity; i++) {
    if (self->buffer[i] == NULL) {
      continue;
    }
    LCH_DictElement *item = self->buffer[i];
    long index = Hash(item->key) % new_capacity;
    while (new_buffer[index] != NULL) {
      index = (index + 1) % new_capacity;
    }
    new_buffer[index] = item;
    LCH_LOG_DEBUG("Moved dict element with key '%s' from index %d to index %d",
                  item->key, i, index);
  }

  LCH_DictElement **old_buffer = self->buffer;
  self->buffer = new_buffer;
  free(old_buffer);
  self->capacity = new_capacity;
  LCH_LOG_DEBUG("Expanded dict buffer. New buffer capacity %d/%d", self->length,
                self->capacity);

  return true;
}

bool LCH_DictSet(LCH_Dict *const self, const char *const key, void *const value,
                 void (*destroy)(void *)) {
  assert(self != NULL);
  assert(key != NULL);

  if (!DictCapacity(self)) {
    return false;
  }

  const size_t hash = Hash(key);
  size_t index = hash % self->capacity;

  while (self->buffer[index] != NULL &&
         strcmp(self->buffer[index]->key, key) != 0) {
    index = (index + 1) % self->capacity;
  }

  if (self->buffer[index] != NULL) {
    assert(self->buffer[index]->key != NULL);
    LCH_DictElement *item = self->buffer[index];
    item->destroy(item->value);
    item->value = value;
    item->destroy = destroy;
    LCH_LOG_DEBUG("Updated value of dict element with key '%s' at index %zu",
                  key, index);
    return true;
  }

  LCH_DictElement *item = (LCH_DictElement *)malloc(sizeof(LCH_DictElement));
  if (item == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict element: %s",
                  strerror(errno));
    return false;
  }

  item->key = strdup(key);
  if (item->key == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict key: %s",
                  strerror(errno));
    free(item);
    return false;
  }
  item->value = value;
  item->destroy = destroy;

  self->buffer[index] = item;
  self->length += 1;
  LCH_LOG_DEBUG("Created dict entry with key '%s' at index %zu. New buffer "
                "capacity %zu/%zu",
                key, index, self->length, self->capacity);

  return true;
}

bool LCH_DictHasKey(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  const size_t hash = Hash(key);
  size_t index = hash % self->capacity;
  while (self->buffer[index] != NULL) {
    if (strcmp(self->buffer[index]->key, key) == 0) {
      LCH_LOG_DEBUG("Found dict entry with key '%s' at index %zu", key,
                    index);
      return true;
    }
    index = (index + 1) % self->capacity;
  }
  LCH_LOG_DEBUG("Did not find dict entry with key '%s'", key);
  return false;
}

void *LCH_DictGet(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  const size_t hash = Hash(key);
  size_t index = hash % self->capacity;
  assert(self->buffer[index] != NULL);
  while (strcmp(self->buffer[index]->key, key) != 0) {
    index = (index + 1) % self->capacity;
    assert(self->buffer[index] != NULL);
  }

  LCH_LOG_DEBUG("Retreived entry from dict with key '%s' at index %zu", key, index);
  return self->buffer[index]->value;
}

void LCH_DictDestroy(LCH_Dict *self) {
  if (self == NULL) {
    return;
  }
  assert(self->buffer != NULL);

  for (size_t i = 0; i < self->capacity; i++) {
    LCH_DictElement *item = self->buffer[i];
    if (item == NULL) {
      continue;
    }
    free(item->key);
    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
    free(item);
    LCH_LOG_DEBUG("Destroyed buffer item at index %zu", i);
  }
  free(self->buffer);
  free(self);
}
