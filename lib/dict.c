#include "dict.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

typedef struct DictElement {
  LCH_Buffer *key;
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

LCH_Dict *LCH_DictCreate() {
  LCH_Dict *self = (LCH_Dict *)malloc(sizeof(LCH_Dict));
  if (self == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memory: %s", strerror(errno));
    return NULL;
  }

  self->length = self->in_use = 0;
  self->capacity = LCH_DICT_CAPACITY;
  self->buffer = (DictElement **)calloc(self->capacity, sizeof(DictElement *));

  if (self->buffer == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memory: %s", strerror(errno));
    free(self);
    return NULL;
  }

  return self;
}

size_t LCH_DictLength(const LCH_Dict *const dict) {
  assert(dict != NULL);
  return dict->length;
}

static size_t HashKey(const LCH_Buffer *const key) {
  assert(key != NULL);

  const char *const buffer = LCH_BufferData(key);
  const size_t length = LCH_BufferLength(key);

  size_t hash = 5381;
  for (size_t i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + (unsigned char)buffer[i];
  }
  return hash;
}

static size_t ComputeIndex(const LCH_Dict *const dict,
                           const LCH_Buffer *const key) {
  assert(dict != NULL);
  assert(dict->buffer != NULL);
  assert(key != NULL);

  size_t index = HashKey(key) % dict->capacity;
  while (true) {
    DictElement *item = dict->buffer[index];
    if (item == NULL) {
      break;
    }
    if (!item->invalidated && LCH_BufferEqual(item->key, key)) {
      break;
    }
    index = (index + 1) % dict->capacity;
  }

  return index;
}

static bool EnsureCapacity(LCH_Dict *const dict) {
  if ((float)dict->in_use < ((float)dict->capacity * LCH_DICT_LOAD_FACTOR)) {
    return true;
  }

  /* If we can free half of the capacity by removing invalidated items, there is
   * no need to expand the buffer. */
  assert(dict->in_use >= dict->length);
  const bool expand = (((float)dict->capacity / 100.f) *
                       (float)(dict->in_use - dict->length)) < 0.5f;

  const size_t new_capacity = (expand) ? dict->capacity * 2 : dict->capacity;
  DictElement **const new_buffer =
      (DictElement **)calloc(new_capacity, sizeof(DictElement *));
  if (new_buffer == NULL) {
    LCH_LOG_ERROR("calloc(3): Failed to allocate memory: %s", strerror(errno));
    return false;
  }

  DictElement **const old_buffer = dict->buffer;
  dict->buffer = new_buffer;
  const size_t old_capacity = dict->capacity;
  dict->capacity = new_capacity;

  for (size_t i = 0; expand && (i < old_capacity); i++) {
    DictElement *const item = old_buffer[i];
    if (item == NULL) {
      continue;
    }

    if (item->invalidated) {
      free(item);
      continue;
    }

    const size_t index = ComputeIndex(dict, item->key);
    assert(new_buffer[index] == NULL);
    new_buffer[index] = item;
  }

  dict->in_use = dict->length;
  free(old_buffer);

  return true;
}

bool LCH_DictSet(LCH_Dict *const dict, const LCH_Buffer *const key,
                 void *const value, void (*destroy)(void *)) {
  assert(dict != NULL);
  assert(dict->buffer != NULL);
  assert(key != NULL);

  if (!EnsureCapacity(dict)) {
    return false;
  }

  const size_t index = ComputeIndex(dict, key);
  if (dict->buffer[index] != NULL) {
    DictElement *const item = dict->buffer[index];
    assert(item->key != NULL);
    assert(LCH_BufferEqual(key, item->key));

    if (item->destroy != NULL) {
      item->destroy(item->value);
    }
    item->value = value;
    item->destroy = destroy;
    return true;
  }

  DictElement *item = (DictElement *)malloc(sizeof(DictElement));
  if (item == NULL) {
    LCH_LOG_ERROR("malloc(3): Failed to allocate memory: %s", strerror(errno));
    return false;
  }

  item->key = LCH_BufferDuplicate(key);
  if (item->key == NULL) {
    free(item);
    return false;
  }
  item->value = value;
  item->destroy = destroy;
  item->invalidated = false;

  dict->buffer[index] = item;
  dict->in_use += 1;
  dict->length += 1;

  return true;
}

void *LCH_DictRemove(LCH_Dict *const dict, const LCH_Buffer *const key) {
  assert(dict != NULL);
  assert(key != NULL);

  const size_t index = ComputeIndex(dict, key);
  DictElement *const item = dict->buffer[index];
  assert(item != NULL);
  assert(item->key != NULL);
  assert(LCH_BufferEqual(item->key, key));
  assert(!item->invalidated);

  LCH_BufferDestroy(item->key);
  item->key = NULL;

  void *value = item->value;
  item->invalidated = true;

  assert(dict->length > 0);
  dict->length -= 1;

  return value;
}

bool LCH_DictHasKey(const LCH_Dict *const dict, const LCH_Buffer *const key) {
  assert(dict != NULL);
  assert(dict->buffer != NULL);
  assert(key != NULL);

  const size_t index = ComputeIndex(dict, key);
  return dict->buffer[index] != NULL;
}

const void *LCH_DictGet(const LCH_Dict *const dict,
                        const LCH_Buffer *const key) {
  assert(dict != NULL);
  assert(dict->buffer != NULL);
  assert(key != NULL);

  const size_t index = ComputeIndex(dict, key);
  DictElement *item = dict->buffer[index];
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

    const LCH_Buffer *const key = item->key;
    assert(key != NULL);

    if (LCH_DictHasKey(other, key)) {
      continue;
    }

    void *value = NULL;
    if (item->value != NULL && duplicate != NULL) {
      value = duplicate(item->value);
      if (value == NULL) {
        LCH_DictDestroy(result);
        return NULL;
      }
    }

    if (!LCH_DictSet(result, key, value, destroy)) {
      destroy(value);
      return NULL;
    }
  }

  return result;
}

LCH_Dict *LCH_DictSetChangedIntersection(const LCH_Dict *const self,
                                         const LCH_Dict *const other,
                                         LCH_DuplicateFn duplicate,
                                         void (*destroy)(void *),
                                         LCH_CompareFn compare) {
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

    const LCH_Buffer *const key = item->key;
    assert(key != NULL);

    if (!LCH_DictHasKey(other, key)) {
      continue;
    }

    const void *const left = item->value;
    const void *const right = LCH_DictGet(other, key);

    if (left == NULL && right == NULL) {
      continue;
    }

    if (left != NULL && right != NULL) {
      if (compare(left, right) == 0) {
        continue;
      }
    }

    if (left != NULL) {
      void *value = duplicate(left);
      if (value == NULL) {
        LCH_DictDestroy(result);
        return NULL;
      }
      if (!LCH_DictSet(result, key, value, destroy)) {
        return NULL;
      }
    } else {
      if (!LCH_DictSet(result, key, NULL, destroy)) {
        return NULL;
      }
    }
  }

  return result;
}

void LCH_DictDestroy(void *const _dict) {
  LCH_Dict *const dict = (LCH_Dict *)_dict;
  if (dict == NULL) {
    return;
  }
  assert(dict->buffer != NULL);

  for (size_t i = 0; i < dict->capacity; i++) {
    DictElement *item = dict->buffer[i];
    if (item == NULL) {
      continue;
    }
    if (!item->invalidated) {
      LCH_BufferDestroy(item->key);
      if (item->destroy != NULL) {
        item->destroy(item->value);
      }
    }
    free(item);
  }
  free(dict->buffer);
  free(dict);
}

LCH_List *LCH_DictGetKeys(const LCH_Dict *const dict) {
  assert(dict != NULL);
  assert(dict->buffer != NULL);

  LCH_List *const keys = LCH_ListCreate();
  for (size_t i = 0; i < dict->capacity; i++) {
    DictElement *const item = dict->buffer[i];
    if (item == NULL || item->invalidated) {
      continue;
    }

    assert(item->key != NULL);
    LCH_Buffer *const key = LCH_BufferDuplicate(item->key);
    if (key == NULL) {
      LCH_ListDestroy(keys);
      return NULL;
    }

    if (!LCH_ListAppend(keys, key, LCH_BufferDestroy)) {
      LCH_BufferDestroy(key);
      LCH_ListDestroy(keys);
      return NULL;
    }
  }

  return keys;
}
