#include <assert.h>
#include <errno.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"

#define INITIAL_CAPACITY 8

typedef struct ListElement {
  void *value;
  void (*destroy)(void *);
} ListElement;

struct LCH_List {
  size_t length;
  size_t capacity;
  ListElement **buffer;
};

LCH_List *LCH_ListCreate() {
  LCH_List *self = (LCH_List *)malloc(sizeof(LCH_List));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list: %s", strerror(errno));
    return NULL;
  }

  self->length = 0;
  self->capacity = INITIAL_CAPACITY;
  self->buffer = (ListElement **)calloc(self->capacity, sizeof(ListElement *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list buffer: %s",
                  strerror(errno));
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

  size_t new_capacity = self->capacity * 2;
  ListElement **new_buffer = (ListElement **)realloc(
      self->buffer, self->capacity * 2 * sizeof(ListElement *));
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("Failed to reallocate memory for list buffer: %s",
                  strerror(errno));
    return false;
  }
  memset(new_buffer + self->capacity, 0, self->capacity);

  self->capacity = new_capacity;
  self->buffer = new_buffer;
  LCH_LOG_DEBUG("Expanded list buffer capacity %d/%d", self->length,
                self->capacity);
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
  ListElement *item = (ListElement *)calloc(1, sizeof(ListElement));
  if (item == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list element: %s",
                  strerror(errno));
    return false;
  }
  item->value = value;
  item->destroy = destroy;

  // Insert item into buffer
  const size_t index = self->length;
  self->buffer[index] = item;
  self->length += 1;
  return true;
}

void *LCH_ListGet(const LCH_List *const self, const size_t index) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(index < self->length);

  ListElement *item = self->buffer[index];
  assert(item != NULL);
  return item->value;
}

void LCH_ListDestroy(LCH_List *self) {
  if (self == NULL) {
    return;
  }
  assert(self->buffer != NULL);

  for (size_t i = 0; i < self->capacity; i++) {
    ListElement *item = self->buffer[i];
    if (item == NULL) {
      continue;
    }
    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
    free(item);
  }
  free(self->buffer);
  free(self);
  LCH_LOG_DEBUG("Destroyed list");
}
