#include <assert.h>
#include <errno.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"

#define INITIAL_CAPACITY 32

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

void LCH_ListSet(LCH_List *const self, const size_t index, void *const value,
                 void (*destroy)(void *)) {
  assert(self != NULL);
  assert(index < self->length);

  self->buffer[index]->value = value;
  self->buffer[index]->destroy = destroy;
}

size_t LCH_ListIndex(const LCH_List *const self, const void *const value,
                     int (*compare)(const void *, const void *)) {
  assert(self != NULL);
  assert(compare != NULL);

  for (size_t i = 0; i < self->length; i++) {
    if (compare(LCH_ListGet(self, i), value) == 0) {
      return i;
    }
  }
  return LCH_ListLength(self);
}

static void Swap(LCH_List *const list, const ssize_t a, const ssize_t b) {
  assert(list != NULL);
  assert(a >= 0);
  assert(a < (ssize_t)list->length);
  assert(b >= 0);
  assert(b < (ssize_t)list->length);

  ListElement *tmp = list->buffer[a];
  list->buffer[a] = list->buffer[b];
  list->buffer[b] = tmp;
}

static size_t Partition(LCH_List *const list, const ssize_t low,
                        const ssize_t high,
                        int (*compare)(const void *, const void *)) {
  void *pivot = LCH_ListGet(list, high);
  ssize_t i = low;
  for (ssize_t j = low; j < high; j++) {
    if (compare(LCH_ListGet(list, j), pivot) <= 0) {
      Swap(list, i++, j);
    }
  }
  Swap(list, i, high);
  return i;
}

static void QuickSort(LCH_List *const list, const ssize_t low,
                      const ssize_t high,
                      int (*compare)(const void *, const void *)) {
  if (low < high) {
    const ssize_t pivot = Partition(list, low, high, compare);
    QuickSort(list, low, pivot - 1, compare);
    QuickSort(list, pivot + 1, high, compare);
  }
}

void LCH_ListSort(LCH_List *const self,
                  int (*compare)(const void *, const void *)) {
  assert(self != NULL);
  QuickSort(self, 0, self->length - 1, compare);
}

void LCH_ListDestroy(LCH_List *self) {
  if (self == NULL) {
    return;
  }
  assert(self->buffer != NULL);

  for (size_t i = 0; i < self->length; i++) {
    ListElement *item = self->buffer[i];
    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
    free(item);
  }
  free(self->buffer);
  free(self);
  LCH_LOG_DEBUG("Destroyed list");
}

void LCH_ListDestroyShallow(LCH_List *self) {
  if (self == NULL) {
    return;
  }
  assert(self->buffer != NULL);

  for (size_t i = 0; i < self->length; i++) {
    ListElement *item = self->buffer[i];
    free(item);
  }

  free(self->buffer);
  free(self);
}
