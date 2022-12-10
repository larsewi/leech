#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "leech.h"
#include "definitions.h"

#define INITIAL_CAPACITY 64

struct LCH_Buffer {
  size_t length;
  size_t capacity;
  char *buffer;
};

LCH_Buffer *LCH_BufferCreate(void) {
  LCH_Buffer *buffer = (LCH_Buffer *)malloc(sizeof(LCH_Buffer));
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for buffer: %s", strerror(errno));
    return NULL;
  }

  buffer->buffer = (char *)malloc(INITIAL_CAPACITY);
  if (buffer->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for buffer string: %s",
                  strerror(errno));
    free(buffer);
    return NULL;
  }

  buffer->length = 0;
  buffer->capacity = INITIAL_CAPACITY;
  buffer->buffer[0] = '\0';

  LCH_LOG_DEBUG("Created string buffer with inital capacity %d/%d",
                buffer->length, buffer->capacity);
  return buffer;
}

bool LCH_BufferAppend(LCH_Buffer *self, const char *format, ...) {
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
    LCH_LOG_DEBUG("Expanded string buffer capacity %zu/%zu", self->length,
                  self->capacity);

    va_start(ap, format);
    int length = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    if (length < 0) {
      LCH_LOG_ERROR("Failed to format string for string buffer: %s",
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
    LCH_LOG_ERROR("Failed to format string for string buffer: %s",
                  strerror(errno));
    return false;
  }

  self->length += (size_t)length;
  return true;
}

size_t LCH_BufferLength(LCH_Buffer *self) {
  assert(self != NULL);
  return self->length;
}

char *LCH_BufferGet(LCH_Buffer *self) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  char *str = strdup(self->buffer);
  if (str == NULL) {
    LCH_LOG_ERROR("Failed to get string from string buffer: %s",
                  strerror(errno));
  }
  return str;
}

void LCH_BufferDestroy(LCH_Buffer *self) {
  free(self->buffer);
  free(self);
  LCH_LOG_DEBUG("Destroyed string buffer");
}
