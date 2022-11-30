#include <assert.h>
#include <errno.h>
#include <string.h>

#include "debug_messenger.h"
#include "definitions.h"
#include "list.h"

#define INITIAL_CAPACITY 8

typedef struct LCH_ListElement {
  void *value;
  void (*destroy)(void *);
} LCH_ListElement;

struct LCH_List {
  size_t length;
  size_t capacity;
  LCH_ListElement **buffer;
};

LCH_List *LCH_ListCreate() {
  LCH_List *self = (LCH_List *)malloc(sizeof(LCH_List));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list: %s", strerror(errno));
    return NULL;
  }

  self->length = 0;
  self->capacity = INITIAL_CAPACITY;
  self->buffer = (LCH_ListElement **)calloc(self->capacity, sizeof(LCH_ListElement *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list buffer: %s", strerror(errno));
    free(self);
    return NULL;
  }

  LCH_LOG_DEBUG("Created list with buffer capacity %d/%d", self->length,
                self->capacity);
  return self;
}

size_t LCH_ListLength(const LCH_List *const self) {
  assert(self != NULL);
  return self->length;
}

static bool ListCapacity(LCH_List *const self) {
  if (self->length < self->capacity) {
    return true;
  }
  self->buffer = (LCH_ListElement **)realloc(self->buffer,
                                      self->capacity * 2 * sizeof(LCH_ListElement *));
  memset(self->buffer + self->capacity, 0, self->capacity);
  self->capacity *= 2;
  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to reallocate memory for list buffer: %s", strerror(errno));
    return false;
  }
  LCH_LOG_DEBUG("Expanded list buffer capacity %d/%d", self->length, self->capacity);
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
  LCH_ListElement *item = (LCH_ListElement *)calloc(1, sizeof(LCH_ListElement));
  if (item == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list element: %s", strerror(errno));
    return false;
  }
  item->value = value;
  item->destroy = destroy;

  // Insert item into buffer
  self->buffer[self->length] = item;
  LCH_LOG_DEBUG("Appended list element to index %d", self->length);
  self->length += 1;

  return true;
}

void *LCH_ListGet(const LCH_List *const self, const size_t index) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(index < self->length);

  LCH_ListElement *item = self->buffer[index];
  return item->value;
}

void LCH_ListDestroy(LCH_List *self) {
  if (self == NULL) {
    return;
  }
  assert(self->buffer != NULL);

  for (size_t i = 0; i < self->capacity; i++) {
    LCH_ListElement *item = self->buffer[i];
    if (item == NULL) {
      continue;
    }
    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
    free(item);
    LCH_LOG_DEBUG("Destroyed list item at index %zu", i);
  }
  free(self->buffer);
  LCH_LOG_DEBUG("Destroyed list buffer");
  free(self);
  LCH_LOG_DEBUG("Destroyed list");
}
