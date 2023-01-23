#include "dict.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"

#define INITIAL_CAPACITY 8
#define LOAD_FACTOR 0.75f

typedef struct DictElement {
  char *key;
  void *value;
  void (*destroy)(void *);
  bool invalidated;
} DictElement;

struct LCH_Dict {
  size_t length;
  size_t capacity;
  size_t in_use;
  DictElement **buffer;
};

struct LCH_DictIter {
  size_t cur_pos;
  const struct LCH_Dict *dict;
};

LCH_Dict *LCH_DictCreate() {
  LCH_Dict *self = (LCH_Dict *)malloc(sizeof(LCH_Dict));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dictionary: %s", strerror(errno));
    return NULL;
  }

  self->length = self->in_use = 0;
  self->capacity = INITIAL_CAPACITY;
  self->buffer = (DictElement **)calloc(self->capacity, sizeof(DictElement *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dictionary buffer: %s",
                  strerror(errno));
    free(self);
    return NULL;
  }

  LCH_LOG_DEBUG("Created dictionary with capacity %d/%d", self->in_use,
                self->capacity);
  return self;
}

size_t LCH_DictLength(const LCH_Dict *const self) {
  assert(self != NULL);
  return self->length;
}

static size_t HashKey(const char *const key) {
  assert(key != NULL);

  size_t hash = 5381, len = strlen(key);
  for (size_t i = 0; i < len; i++) {
    hash = ((hash << 5) + hash) + key[i];
  }

  return hash;
}

static size_t ComputeIndex(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(key != NULL);

  size_t index = HashKey(key) % self->capacity;
  while ((self->buffer[index] != NULL) && !(self->buffer[index]->invalidated) &&
         (strcmp(self->buffer[index]->key, key) != 0)) {
    index = (index + 1) % self->capacity;
  }
  return index;
}

static bool EnsureCapacity(LCH_Dict *const self) {
  if (self->in_use < (self->capacity * LOAD_FACTOR)) {
    return true;
  }

  /* If we can free (1.f - LOAD_FACTOR) of the capacity by removing invalidated
   * items, there is no need to expand the buffer. */
  const bool expand = ((self->capacity / 100.f) * (self->in_use - self->length)) > (1.f - LOAD_FACTOR);
  const size_t new_capacity = (expand) ? self->capacity : self->capacity * 2;

  DictElement **new_buffer =
      (DictElement **)calloc(new_capacity, sizeof(DictElement *));
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for dict element: %s",
                  strerror(errno));
    return false;
  }

  DictElement **const old_buffer = self->buffer;
  self->buffer = new_buffer;
  const size_t old_capacity = self->capacity;
  self->capacity = new_capacity;

  for (size_t i = 0; i < old_capacity; i++) {
    DictElement *const item = old_buffer[i];

    if (item == NULL) {
      continue;
    }

    if (item->invalidated) {
      free(item);
      continue;
    }

    const size_t index = ComputeIndex(self, item->key);
    assert(new_buffer[index] == NULL);
    new_buffer[index] = item;
  }

  self->in_use = self->length;
  free(old_buffer);

  LCH_LOG_DEBUG("%s. New buffer capacity %zu/%zu.", (expand) ? "Expanded dict" : "Cleaned invalidated items", self->in_use, self->capacity);

  return true;
}

bool LCH_DictSet(LCH_Dict *const self, const char *const key, void *const value,
                 void (*destroy)(void *)) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(key != NULL);

  if (!EnsureCapacity(self)) {
    return false;
  }

  const size_t index = ComputeIndex(self, key);
  if (self->buffer[index] != NULL) {
    DictElement *const item = self->buffer[index];
    assert(item->key != NULL);
    assert(strcmp(item->key, key) == 0);

    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
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
  item->invalidated = false;

  self->buffer[index] = item;
  self->in_use += 1;
  self->length += 1;

  return true;
}

void *LCH_DictRemove(LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(key != NULL);

  const size_t index = ComputeIndex(self, key);
  DictElement *const item = self->buffer[index];
  assert(item != NULL);

  free(item->key);

  void *value = item->value;
  item->invalidated = true;

  assert(self->length > 0);
  self->length -= 1;

  return value;
}

bool LCH_DictHasKey(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(key != NULL);

  const size_t index = ComputeIndex(self, key);
  return self->buffer[index] != NULL;
}

void *LCH_DictGet(const LCH_Dict *const self, const char *const key) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(key != NULL);

  const size_t index = ComputeIndex(self, key);
  DictElement *item = self->buffer[index];
  assert(item != NULL);
  return item->value;
}

LCH_Dict *LCH_DictSetMinus(const LCH_Dict *const self,
                           const LCH_Dict *const other,
                           void *(*duplicate)(const void *),
                           void (*destroy)(void *)) {
  assert(self != NULL);
  assert(other != NULL);
  assert(self->buffer != NULL);

  LCH_Dict *result = LCH_DictCreate();
  if (result == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < self->capacity; i++) {
    const DictElement *const item = self->buffer[i];

    if (self->buffer[i] == NULL) {
      continue;
    }

    const char *const key = item->key;
    assert(key != NULL);

    if (LCH_DictHasKey(other, key)) {
      continue;
    }

    void *value = duplicate(item->value);
    if (value == NULL) {
      LCH_DictDestroy(result);
      LCH_LOG_ERROR("Failed to duplicate value from dict entry at key '%s'.",
                    key);
      return NULL;
    }

    if (!LCH_DictSet(result, key, value, destroy)) {
      return NULL;
    }
  }

  return result;
}

LCH_Dict *LCH_DictSetChangedIntersection(
    const LCH_Dict *const self, const LCH_Dict *const other,
    void *(*duplicate)(const void *), void (*destroy)(void *),
    int (*compare)(const void *, const void *)) {
  assert(self != NULL);
  assert(other != NULL);
  assert(self->buffer != NULL);

  LCH_Dict *result = LCH_DictCreate();
  if (result == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < self->capacity; i++) {
    const DictElement *const item = self->buffer[i];

    if (self->buffer[i] == NULL) {
      continue;
    }

    const char *const key = item->key;
    assert(key != NULL);

    if (!LCH_DictHasKey(other, key)) {
      continue;
    }

    if (compare(LCH_DictGet(self, key), LCH_DictGet(other, key)) == 0) {
      continue;
    }

    void *value = duplicate(item->value);
    if (value == NULL) {
      LCH_DictDestroy(result);
      LCH_LOG_ERROR("Failed to duplicate value from dict entry at key '%s'.",
                    key);
      return NULL;
    }

    if (!LCH_DictSet(result, key, value, destroy)) {
      return NULL;
    }
  }

  return result;
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
    if (!item->invalidated) {
      free(item->key);
      if (item->destroy != NULL) {
        item->destroy(item->value);
      }
    }
    free(item);
  }
  free(self->buffer);
  free(self);
}

LCH_DictIter *LCH_DictIterCreate(const LCH_Dict *const dict) {
  assert(dict != NULL);

  LCH_DictIter *iter = malloc(sizeof(LCH_DictIter));
  if (iter == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for iterator: %s",
                  strerror(errno));
    return NULL;
  }
  iter->cur_pos = 0;
  iter->dict = dict;

  return iter;
}

bool LCH_DictIterHasNext(LCH_DictIter *const iter) {
  assert(iter != NULL);
  assert(iter->dict != NULL);
  assert(iter->dict->buffer != NULL);

  DictElement **buffer = iter->dict->buffer;
  while (iter->cur_pos < iter->dict->capacity) {
    if (buffer[iter->cur_pos] == NULL || buffer[iter->cur_pos]->invalidated) {
      iter->cur_pos += 1;
      continue;
    }
    iter->cur_pos += 1;
    return true;
  }
  return false;
}

char *LCH_DictIterGetKey(const LCH_DictIter *const iter) {
  assert(iter != NULL);
  assert(iter->dict != NULL);
  assert(iter->dict->buffer != NULL);

  DictElement *item = iter->dict->buffer[iter->cur_pos - 1];
  assert(item != NULL);
  assert(item->key != NULL);
  return item->key;
}

void *LCH_DictIterGetValue(const LCH_DictIter *const iter) {
  assert(iter != NULL);
  assert(iter->dict != NULL);
  assert(iter->dict->buffer != NULL);

  DictElement *item = iter->dict->buffer[iter->cur_pos - 1];
  assert(item != NULL);
  return item->value;
}
