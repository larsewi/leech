#include "buffer.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"

#define INITIAL_CAPACITY 1024

struct LCH_Buffer {
  size_t length;
  size_t capacity;
  char *buffer;
};

static bool EnsureCapacity(LCH_Buffer *const self, const size_t needed) {
  assert(self != NULL);

  if ((self->capacity - self->length) <= needed) {
    size_t new_capacity = self->capacity * 2;
    char *new_buffer = realloc(self->buffer, new_capacity);
    if (new_buffer == NULL) {
      LCH_LOG_ERROR("Failed to reallocate memory for buffer: %s", strerror(errno));
      return false;
    }

    self->capacity = new_capacity;
    self->buffer = new_buffer;

    LCH_LOG_DEBUG("Expanded buffer capacity %zu/%zu", self->length,
                  self->capacity);
  }

  return true;
}

LCH_Buffer *LCH_BufferCreate(void) {
  LCH_Buffer *self = (LCH_Buffer *)malloc(sizeof(LCH_Buffer));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for buffer: %s", strerror(errno));
    return NULL;
  }

  self->buffer = (char *)malloc(INITIAL_CAPACITY);
  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for buffer: %s", strerror(errno));
    free(self);
    return NULL;
  }

  self->length = 0;
  self->capacity = INITIAL_CAPACITY;
  self->buffer[0] = '\0';

  LCH_LOG_DEBUG("Created string buffer with inital capacity %d/%d",
                self->length, self->capacity);
  return self;
}

bool LCH_BufferPrintFormat(LCH_Buffer *const self, const char *const format, ...) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(format != NULL);

  va_list ap;

  va_start(ap, format);
  int length = vsnprintf(NULL, 0, format, ap);
  va_end(ap);

  if (length < 0) {
    LCH_LOG_ERROR("Failed to format string for string buffer: %s",
                  strerror(errno));
    self->buffer[self->length] = '\0';
    return false;
  }

  while ((size_t)length >= self->capacity - self->length) {
    const size_t new_capacity = self->capacity * 2;
    char *new_buffer = (char *)realloc((void *)self->buffer, new_capacity);

    if (new_buffer == NULL) {
      LCH_LOG_ERROR("Failed to reallocate memory for string buffer: %s",
                    strerror(errno));
      return false;
    }

    self->capacity = new_capacity;
    self->buffer = new_buffer;
    LCH_LOG_DEBUG("Expanded buffer capacity %zu/%zu", self->length,
                  self->capacity);

    va_start(ap, format);
    int length = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    if (length < 0) {
      LCH_LOG_ERROR("Failed to format string for buffer: %s",
                    strerror(errno));
      self->buffer[self->length] = '\0';
      return false;
    }
  }

  va_start(ap, format);
  length = vsnprintf(self->buffer + self->length, self->capacity - self->length,
                     format, ap);
  va_end(ap);

  if (length < 0) {
    self->buffer[self->length] = '\0';
    LCH_LOG_ERROR("Failed to format string for buffer: %s",
                  strerror(errno));
    return false;
  }

  self->length += (size_t)length;
  return true;
}

size_t LCH_BufferLength(LCH_Buffer *const self) {
  assert(self != NULL);
  return self->length;
}

char *LCH_BufferStringDup(LCH_Buffer *const self) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  char *str = strdup(self->buffer);
  if (str == NULL) {
    LCH_LOG_ERROR("Failed to get string from string buffer: %s",
                  strerror(errno));
  }
  return str;
}

char *LCH_BufferGet(LCH_Buffer *const self) {
  return self->buffer;
}

bool LCH_BufferAppendLong(LCH_Buffer *const self, const uint32_t value) {
  assert(self != NULL);
  assert(self->buffer != NULL);

  if (!EnsureCapacity(self, sizeof(uint32_t))) {
    return false;
  }

  uint32_t *pos = (uint32_t *) &(self->buffer[self->length]);
  *pos = value;
  self->length += sizeof(uint32_t);
  self->buffer[self->length] = '\0';

  return true;
}

uint32_t *LCH_BufferAllocateLong(LCH_Buffer *const self) {
  assert(self != NULL);
  assert(self->buffer != NULL);

  if (!EnsureCapacity(self, sizeof(uint32_t))) {
    return NULL;
  }

  self->length += sizeof(uint32_t);
  self->buffer[self->length] = '\0';

  return (uint32_t *) &(self->buffer[self->length]);
}

void LCH_BufferDestroy(LCH_Buffer *const self) {
  if (self != NULL) {
    assert(self->buffer != NULL);
    free(self->buffer);

    free(self);
  }
}
