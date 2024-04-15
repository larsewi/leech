#include "buffer.h"

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "files.h"
#include "logger.h"
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

static LCH_Buffer *LCH_BufferCreateWithCapacity(size_t capacity) {
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

const char *LCH_BufferData(const LCH_Buffer *const buffer) {
  assert(buffer != NULL);
  assert(buffer->buffer != NULL);

  return buffer->buffer;
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
    if (sscanf(hex->buffer + (i * 2), "%2hhx",
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
      LCH_LOG_ERROR(
          "Failed to convert unicode escape sequence to UTF8:\n"
          "%.4s\n%*s^ Not a hexadecimal number!",
          in, i);
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

  uint16_t *host = (uint16_t *)LCH_BufferData(bytes);
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

LCH_Buffer *LCH_BufferFromString(const char *const str) {
  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return NULL;
  }

  const size_t len = strlen(str);
  if (!EnsureCapacity(buffer, len)) {
    LCH_BufferDestroy(buffer);
    return NULL;
  }

  for (size_t i = 0; i < len; i++) {
    buffer->buffer[buffer->length++] = str[i];
  }
  buffer->buffer[buffer->length] = '\0';

  return buffer;
}

const LCH_Buffer *LCH_BufferStaticFromString(const char *const str) {
  static LCH_Buffer buffer;
  buffer.buffer = (char *)str;
  buffer.length = strlen(str);
  buffer.capacity = 0;
  return &buffer;
}

bool LCH_BufferWriteFile(const LCH_Buffer *buffer, const char *filename) {
  assert(buffer != NULL);
  assert(filename != NULL);

  if (!LCH_FileCreateParentDirectories(filename)) {
    return false;
  }

  const int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, (mode_t)0600);
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
  LCH_LOG_DEBUG("Wrote %zu bytes to file '%s'", tot_written, filename);

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

void LCH_BufferTrim(LCH_Buffer *const buffer, const char ch) {
  assert(buffer != NULL);
  assert(buffer->buffer != NULL);

  size_t start = 0;
  while (start < buffer->length && buffer->buffer[start] == ch) {
    start += 1;
  }

  size_t end = buffer->length;
  while (end > start && buffer->buffer[end - 1] == ch) {
    end -= 1;
  }

  buffer->length = end - start;
  memmove(buffer->buffer, buffer->buffer + start, buffer->length);
  buffer->buffer[buffer->length] = '\0';
}

bool LCH_BufferAppendBuffer(LCH_Buffer *const self,
                            const LCH_Buffer *const other) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(other != NULL);
  assert(other->buffer != NULL);

  const size_t other_length = other->length;
  if (!EnsureCapacity(self, other_length)) {
    return false;
  }

  for (size_t i = 0; i < other_length; i++) {
    self->buffer[self->length + i] = other->buffer[i];
  }
  self->length += other_length;
  self->buffer[self->length] = '\0';

  return true;
}

bool LCH_BufferEqual(const LCH_Buffer *const self,
                     const LCH_Buffer *const other) {
  assert(self != NULL);
  assert(other != NULL);

  const bool equal = LCH_BufferCompare(self, other) == 0;
  return equal;
}

int LCH_BufferCompare(const LCH_Buffer *self, const LCH_Buffer *other) {
  assert(self != NULL);
  assert(self->buffer != NULL);
  assert(other != NULL);
  assert(other->buffer != NULL);

  if (self->length < other->length) {
    return -1;
  }
  if (self->length > other->length) {
    return 1;
  }

  assert(self->length == other->length);
  const int ret = memcmp(self->buffer, other->buffer, self->length);
  if (ret < 0) {
    return -1;
  }
  if (ret > 0) {
    return 1;
  }
  return 0;
}

LCH_Buffer *LCH_BufferDuplicate(const LCH_Buffer *const original) {
  LCH_Buffer *const duplicate = LCH_BufferCreateWithCapacity(original->length);
  if (duplicate == NULL) {
    return NULL;
  }

  memcpy(duplicate->buffer, original->buffer,
         original->length + 1 /* NULL-byte */);
  duplicate->length = original->length;
  return duplicate;
}
