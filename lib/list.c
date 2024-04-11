#include "list.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"

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

static bool EnsureCapacity(LCH_List *const self, const size_t n_items) {
  assert(self != NULL);

  size_t new_capacity = self->capacity;
  while (new_capacity < self->length + n_items) {
    new_capacity *= 2;
  }

  if (new_capacity == self->capacity) {
    return true;
  }

  ListElement **new_buffer = (ListElement **)realloc(
      self->buffer, sizeof(ListElement *) * new_capacity);
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("Failed to expand list buffer from %zu to %zu elements: %s",
                  self->capacity, new_capacity, strerror(errno));
    return false;
  }

  self->capacity = new_capacity;
  self->buffer = new_buffer;

  return true;
}

static LCH_List *LCH_ListCreateWithCapacity(const size_t capacity) {
  LCH_List *self = (LCH_List *)malloc(sizeof(LCH_List));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list: %s", strerror(errno));
    return NULL;
  }

  self->length = 0;
  self->capacity = capacity;
  self->buffer = (ListElement **)calloc(self->capacity, sizeof(ListElement *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for list buffer: %s",
                  strerror(errno));
    free(self);
    return NULL;
  }

  return self;
}

LCH_List *LCH_ListCreate() {
  return LCH_ListCreateWithCapacity(INITIAL_CAPACITY);
}

size_t LCH_ListLength(const LCH_List *const self) {
  assert(self != NULL);
  return self->length;
}

bool LCH_ListAppend(LCH_List *const self, void *const value,
                    void (*destroy)(void *)) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(self->capacity >= self->length);

  if (!EnsureCapacity(self, 1)) {
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

bool LCH_ListAppendStringDuplicate(LCH_List *const list,
                                   const char *const str) {
  assert(list != NULL);
  assert(str != NULL);

  char *const dup = strdup(str);
  if (dup == NULL) {
    LCH_LOG_ERROR("Failed to duplicate string: %s", strerror(errno));
    return false;
  }

  return LCH_ListAppend(list, dup, free);
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

  self->buffer[index]->destroy(self->buffer[index]->value);
  self->buffer[index]->value = value;
  self->buffer[index]->destroy = destroy;
}

size_t LCH_ListIndex(const LCH_List *const self, const void *const value,
                     LCH_CompareFn compare) {
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
                        const ssize_t high, LCH_CompareFn compare) {
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
                      const ssize_t high, LCH_CompareFn compare) {
  if (low < high) {
    const ssize_t pivot = Partition(list, low, high, compare);
    QuickSort(list, low, pivot - 1, compare);
    QuickSort(list, pivot + 1, high, compare);
  }
}

void LCH_ListSort(LCH_List *const self, LCH_CompareFn compare) {
  assert(self != NULL);
  QuickSort(self, 0, self->length - 1, compare);
}

void LCH_ListDestroy(void *const self) {
  LCH_List *const list = (LCH_List *)self;
  if (list == NULL) {
    return;
  }

  assert(list->buffer != NULL);

  for (size_t i = 0; i < list->length; i++) {
    ListElement *item = list->buffer[i];
    assert(item != NULL);
    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
    free(item);
  }
  free(list->buffer);
  free(list);
}

void *LCH_ListRemove(LCH_List *const list, const size_t index) {
  assert(list != NULL);
  assert(list->buffer != NULL);
  assert(list->length > index);

  ListElement *const element = list->buffer[index];
  void *value = element->value;
  free(element);

  list->length -= 1;
  for (size_t i = index; i < list->length; i++) {
    list->buffer[i] = list->buffer[i + 1];
  }
  return value;
}

LCH_List *LCH_ListCopy(const LCH_List *const original, LCH_DuplicateFn copy_fn,
                       void (*destroy_fn)(void *)) {
  assert(original != NULL);
  assert(original->buffer != NULL);

  LCH_List *const copy = LCH_ListCreateWithCapacity(original->length);
  if (copy == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < original->length; i++) {
    void *orig_element = LCH_ListGet(original, i);
    void *copy_element = copy_fn(orig_element);
    if (copy_element == NULL) {
      LCH_ListDestroy(copy);
      return NULL;
    }

    if (!LCH_ListAppend(copy, copy_element, destroy_fn)) {
      destroy_fn(copy_element);
      LCH_ListDestroy(copy);
      return NULL;
    }
  }

  return copy;
}

bool LCH_ListInsert(LCH_List *const list, const size_t index, void *const value,
                    void (*destroy)(void *)) {
  assert(list->buffer != NULL);
  assert(list->length >= index);

  if (!EnsureCapacity(list, 1)) {
    return false;
  }

  ListElement *const element = (ListElement *)malloc(sizeof(ListElement));
  if (element == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memory: %s", strerror(errno));
    return false;
  }
  element->value = value;
  element->destroy = destroy;

  memmove(list->buffer + index + 1, list->buffer + index,
          (list->length - index) * sizeof(ListElement *));
  list->buffer[index] = element;
  list->length += 1;
  return true;
}
