#include "buffer.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "definitions.h"
#include "leech.h"

#define INITIAL_CAPACITY 255

struct LCH_Buffer {
  size_t length;
  size_t capacity;
  char *buffer;
};

static bool EnsureCapacity(LCH_Buffer *const self, const size_t needed) {
  assert(self != NULL);

  while ((self->capacity - self->length) <= needed) {
    size_t new_capacity = self->capacity * 2;
    char *new_buffer = (char *)realloc(self->buffer, new_capacity);
    if (new_buffer == NULL) {
      LCH_LOG_ERROR("Failed to reallocate memory for buffer: %s",
                    strerror(errno));
      return false;
    }

    self->capacity = new_capacity;
    self->buffer = new_buffer;
  }

  return true;
}

LCH_Buffer *LCH_BufferCreateWithCapacity(size_t capacity) {
  LCH_Buffer *self = (LCH_Buffer *)malloc(sizeof(LCH_Buffer));
  if (self == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for buffer: %s", strerror(errno));
    return NULL;
  }

  self->capacity = capacity + 1;
  self->length = 0;
  self->buffer = (char *)malloc(self->capacity);
  if (self->buffer == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for buffer: %s", strerror(errno));
    free(self);
    return NULL;
  }
  self->buffer[0] = '\0';

  return self;
}

LCH_Buffer *LCH_BufferCreate(void) {
  return LCH_BufferCreateWithCapacity(INITIAL_CAPACITY);
}

bool LCH_BufferAppend(LCH_Buffer *const self, const char byte) {
  if (!EnsureCapacity(self, 1)) {
    return false;
  }

  self->buffer[self->length++] = byte;
  self->buffer[self->length] = '\0';
  return true;
}

bool LCH_BufferPrintFormat(LCH_Buffer *const self, const char *const format,
                           ...) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(format != NULL);

  va_list ap;

  /* TODO:
   * Concerning the return value of snprintf(), SUSv2 and C99 contradict each
   * other: when snprintf() is called with size=0 then SUSv2 stipulates an
   * unspecified return value less than 1, while C99 allows str to be NULL in
   * this case, and gives the return value (as always) as the number of
   * characters that would have been written in case the output string has
   * been large enough. */

  // Figure out how many bytes we need
  va_start(ap, format);
  const int length = vsnprintf(NULL, 0, format, ap);
  assert(length >= 0);
  va_end(ap);
  if (length < 0) {
    LCH_LOG_ERROR(
        "Failed to calulate length needed to print formatted string to buffer: "
        "%s",
        strerror(errno));
    self->buffer[self->length] = '\0';
    return false;
  }

  if (!EnsureCapacity(self, (size_t)length)) {
    return false;
  }

  va_start(ap, format);
  const int ret = vsnprintf(self->buffer + self->length,
                            self->capacity - self->length, format, ap);
  assert(length >= 0);
  va_end(ap);
  if (length < 0) {
    LCH_LOG_ERROR("Failed to print formatted string to buffer: %s",
                  strerror(errno));
    self->buffer[self->length] = '\0';
    return false;
  }
  assert(ret == length);
  assert((size_t)ret < self->capacity - self->length);

  self->length += (size_t)length;
  return true;
}

size_t LCH_BufferLength(const LCH_Buffer *const self) {
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

const void *LCH_BufferGet(const LCH_Buffer *const self, const size_t offset) {
  assert(self->length >= offset);
  return self->buffer + offset;
}

bool LCH_BufferAllocate(LCH_Buffer *const self, const size_t size,
                        size_t *const offset) {
  assert(self != NULL);
  assert(self->buffer != NULL);

  if (!EnsureCapacity(self, size)) {
    return false;
  }

  *offset = self->length;
  memset(self->buffer + self->length, 0, size + 1);
  self->length += size;
  return true;
}

void LCH_BufferDestroy(LCH_Buffer *const self) {
  if (self != NULL) {
    assert(self->buffer != NULL);
    free(self->buffer);

    free(self);
  }
}

void LCH_BufferSet(LCH_Buffer *const self, const size_t offset,
                   const void *const value, const size_t size) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(self->capacity > offset + size);
  memcpy(self->buffer + offset, value, size);
}

bool LCH_BufferHexDump(LCH_Buffer *const hex, const LCH_Buffer *const bin) {
  assert(hex != NULL);
  assert(bin != NULL);
  assert(bin->buffer != NULL);

  for (size_t i = 0; i < bin->length; i++) {
    if (!LCH_BufferPrintFormat(hex, "%02x", bin->buffer[i])) {
      return false;
    }
  }
  return true;
}

bool LCH_BufferBinDump(LCH_Buffer *const bin, const LCH_Buffer *const hex) {
  assert(bin != NULL);
  assert(hex != NULL);

  if (hex->length % 2 != 0) {
    LCH_LOG_WARNING(
        "Performing binary dump with an odd number of hexadecimal characters: "
        "Last byte will be stripped.");
  }

  size_t num_bytes = hex->length / 2;
  if (!EnsureCapacity(bin, num_bytes)) {
    return false;
  }

  for (size_t i = 0; i < num_bytes; i++) {
    if (sscanf((const char *)LCH_BufferGet(hex, i * 2), "%2hhx",
               bin->buffer + (bin->length + i)) != 1) {
      bin->buffer[bin->length] = '\0';
      return false;
    }
  }
  bin->length += num_bytes;
  bin->buffer[bin->length] = '\0';

  return true;
}

void LCH_BufferChop(LCH_Buffer *const self, size_t offset) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(offset <= self->length);
  self->buffer[offset] = '\0';
  self->length = offset;
}

void LCH_BufferDestroyShallow(LCH_Buffer *const self) { free(self); }

char *LCH_BufferToString(LCH_Buffer *const self) {
  char *str = self->buffer;
  free(self);
  return str;
}
