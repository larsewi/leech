#include <assert.h>
#include <errno.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"
#include "dict.h"

#define INITIAL_CAPACITY 8
#define LOAD_FACTOR 0.75f

typedef struct DictElement {
  char *key;
  void *value;
  void (*destroy)(void *);
} DictElement;

struct LCH_Dict {
  size_t length;
  size_t capacity;
  DictElement **buffer;
};

LCH_Dict *LCH_DictCreate() {
  LCH_Dict *self = (LCH_Dict *)malloc(sizeof(LCH_Dict));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict: %s", strerror(errno));
    return NULL;
  }

  self->length = 0;
  self->capacity = INITIAL_CAPACITY;
  self->buffer = (DictElement **)calloc(self->capacity, sizeof(DictElement *));

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
  DictElement **new_buffer =
      (DictElement **)calloc(new_capacity, sizeof(DictElement *));
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict element: %s",
                  strerror(errno));
    return false;
  }

  for (size_t i = 0; i < self->capacity; i++) {
    if (self->buffer[i] == NULL) {
      continue;
    }
    DictElement *item = self->buffer[i];
    long index = Hash(item->key) % new_capacity;
    while (new_buffer[index] != NULL) {
      index = (index + 1) % new_capacity;
    }
    new_buffer[index] = item;
  }

  DictElement **old_buffer = self->buffer;
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
    DictElement *item = self->buffer[index];
    item->destroy(item->value);
    item->value = value;
    item->destroy = destroy;
    return true;
  }

  DictElement *item = (DictElement *)malloc(sizeof(DictElement));
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

  return true;
}

bool LCH_DictHasKey(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  const size_t hash = Hash(key);
  size_t index = hash % self->capacity;
  while (self->buffer[index] != NULL) {
    if (strcmp(self->buffer[index]->key, key) == 0) {
      return true;
    }
    index = (index + 1) % self->capacity;
  }
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
  return self->buffer[index]->value;
}

void LCH_DictDestroy(LCH_Dict *self) {
  if (self == NULL) {
    return;
  }
  assert(self->buffer != NULL);

  for (size_t i = 0; i < self->capacity; i++) {
    DictElement *item = self->buffer[i];
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
  LCH_LOG_DEBUG("Destroyed dict");
}
