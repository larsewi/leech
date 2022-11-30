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
  self->buffer = (LCH_DictElement **)calloc(self->capacity, sizeof(LCH_DictElement *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict buffer: %s", strerror(errno));
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
  if (self->length < self->capacity / LOAD_FACTOR) {
    return true;
  }

  size_t new_capacity = self->capacity * 2;
  LCH_DictElement **new_buffer = (LCH_DictElement **)calloc(new_capacity, sizeof(LCH_DictElement *));
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory: %s", strerror(errno));
    return false;
  }

  for (size_t i = 0; i < self->capacity; i++) {
    if (self->buffer[i] == NULL) {
      continue;
    }
    LCH_DictElement *item = self->buffer[i];

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

  const size_t hash = Hash(key);
  size_t index = hash % self->capacity;

  while (self->buffer[index] != NULL &&
         strcmp(self->buffer[index]->key, key) != 0) {
    index++;
  }

  if (self->buffer[index] != NULL) {
    assert(self->buffer[index]->key != NULL);
    LCH_DictElement *item = self->buffer[index];
    item->destroy(item->value);
    item->value = value;
    item->destroy = destroy;
    return true;
  }

  LCH_DictElement *item = (LCH_DictElement *)calloc(1, sizeof(LCH_DictElement));
  if (item == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict element: %s", strerror(errno));
    return false;
  }

  item->key = strdup(key);
  if (item->key == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict key: %s", strerror(errno));
    free(item);
    return false;
  }
  item->value = value;
  item->destroy = destroy;

  self->buffer[index] = item;
  LCH_LOG_DEBUG("Set entry to dict with key '%s', hash %zu, index %zu", key,
                hash, index);

  return true;
}

bool LCH_DictHasKey(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  const size_t hash = Hash(key);
  size_t index = hash % self->capacity;
  while (self->buffer[index] != NULL) {
    if (strcmp(self->buffer[index]->key, key) == 0) {
      LCH_LOG_DEBUG("Found entry in dict with key '%s', hash %zu, index %zu",
                    key, hash, index);
      return true;
    }
    index += 1;
  }
  LCH_LOG_DEBUG("Did not find entry in dict with key '%s', hash %zu", key,
                hash);
  return false;
}

void *LCH_DictGet(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  const size_t hash = Hash(key);
  size_t index = hash % self->capacity;
  while (self->buffer[index] != NULL &&
         strcmp(self->buffer[index]->key, key) != 0) {
    index += 1;
  }
  assert(self->buffer[index] != NULL);

  LCH_LOG_DEBUG("Get entry from dict with key '%s', hash %zu, index %zu", key,
                hash, index);
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
