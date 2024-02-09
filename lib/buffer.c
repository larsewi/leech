#include "buffer.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "definitions.h"
#include "leech.h"
#include "utils.h"

#define INITIAL_CAPACITY 1028

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

void LCH_BufferDestroy(void *const self) {
  LCH_Buffer *const buffer = (LCH_Buffer *)self;
  if (buffer != NULL) {
    assert(buffer->buffer != NULL);
    free(buffer->buffer);

    free(buffer);
  }
}

void LCH_BufferSet(LCH_Buffer *const self, const size_t offset,
                   const void *const value, const size_t size) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(self->capacity > offset + size);
  memcpy(self->buffer + offset, value, size);
}

bool LCH_BufferBytesToHex(LCH_Buffer *const hex,
                          const LCH_Buffer *const bytes) {
  assert(hex != NULL);
  assert(bytes != NULL);
  assert(bytes->buffer != NULL);

  for (size_t i = 0; i < bytes->length; i++) {
    if (!LCH_BufferPrintFormat(hex, "%02x", (unsigned char)bytes->buffer[i])) {
      return false;
    }
  }
  return true;
}

bool LCH_BufferHexToBytes(LCH_Buffer *const bytes,
                          const LCH_Buffer *const hex) {
  assert(bytes != NULL);
  assert(hex != NULL);
  assert(hex->length % 2 == 0);  // Illegal: Odd number of hexadecimals

  size_t num_bytes = hex->length / 2;
  if (!EnsureCapacity(bytes, num_bytes)) {
    return false;
  }

  for (size_t i = 0; i < num_bytes; i++) {
    if (sscanf((const char *)LCH_BufferGet(hex, i * 2), "%2hhx",
               bytes->buffer + (bytes->length + i)) != 1) {
      bytes->buffer[bytes->length] = '\0';
      return false;
    }
  }
  bytes->length += num_bytes;
  bytes->buffer[bytes->length] = '\0';

  return true;
}

bool LCH_BufferUnicodeToUTF8(LCH_Buffer *const buffer, const char *const in) {
  LCH_Buffer *hex = LCH_BufferCreate();
  for (size_t i = 0; i < 4; i++) {
    if (!isxdigit(in[i])) {
      LCH_BufferDestroy(hex);
      return false;
    }
    if (!LCH_BufferAppend(hex, in[i])) {
      LCH_BufferDestroy(hex);
      return false;
    }
  }

  LCH_Buffer *bytes = LCH_BufferCreateWithCapacity(4);
  if (!LCH_BufferHexToBytes(bytes, hex)) {
    LCH_BufferDestroy(hex);
    return false;
  }
  LCH_BufferDestroy(hex);

  uint16_t *host = (uint16_t *)LCH_BufferGet(bytes, 0);
  assert(host != NULL);
  uint16_t code_point = htons(*host);
  LCH_BufferDestroy(bytes);

  if (code_point < 0x80) {
    if (!LCH_BufferAppend(buffer, (unsigned char)code_point)) {
      return false;
    }
  } else if (code_point < 0x800) {
    if (!LCH_BufferAppend(buffer, 192 + code_point / 64)) {
      return false;
    }
    if (!LCH_BufferAppend(buffer, 128 + code_point % 64)) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

void LCH_BufferChop(LCH_Buffer *const self, size_t offset) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(offset <= self->length);
  self->buffer[offset] = '\0';
  self->length = offset;
}

char *LCH_BufferToString(LCH_Buffer *const self) {
  char *str = self->buffer;
  free(self);
  return str;
}

bool LCH_BufferWriteFile(const LCH_Buffer *buffer, const char *filename) {
  assert(buffer != NULL);
  assert(filename != NULL);

  if (!LCH_CreateParentDirectories(filename)) {
    LCH_LOG_ERROR("Failed to create parent directories for file '%s'",
                  filename);
    return false;
  }

  const int fd = open(filename, O_WRONLY);
  if (fd == -1) {
    LCH_LOG_ERROR("Failed to open file '%s' for writing: %s", filename,
                  strerror(errno));
    return false;
  }

  size_t tot_written = 0;
  while (tot_written < buffer->length) {
    ssize_t n_written = write(fd, buffer->buffer, buffer->length);
    if (n_written < 0) {
      LCH_LOG_ERROR("Failed to write to file '%s': %s", filename,
                    strerror(errno));
      close(fd);
      return false;
    }

    tot_written += n_written;
  }

  close(fd);
  LCH_LOG_DEBUG("Wrote %zu bytes to file '%s'", filename);

  return true;
}

bool LCH_BufferReadFile(LCH_Buffer *const buffer, const char *const filename) {
  assert(buffer != NULL);
  assert(filename != NULL);

  const int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    LCH_LOG_ERROR("Failed to open file '%s' for reading: %s", filename,
                  strerror(errno));
    return false;
  }

  const size_t to_read = 4096;
  ssize_t n_read = 0;
  do {
    if (!EnsureCapacity(buffer, to_read)) {
      close(fd);
      return false;
    }

    n_read = read(fd, buffer->buffer + buffer->length, to_read);
    if (n_read < 0) {
      LCH_LOG_ERROR("Failed to read file '%s': %s", filename, strerror(errno));
      close(fd);
      return false;
    }

    buffer->length += (size_t)n_read;
  } while (n_read > 0);

  close(fd);
  buffer->buffer[buffer->length] = '\0';
  LCH_LOG_DEBUG("Read %zu bytes from file '%s'", buffer->length, filename);

  return true;
}
