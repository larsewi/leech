#include <assert.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <string.h>

#include "leech.h"

LCH_List *LCH_SplitString(const char *str, const char *del) {
  LCH_List *list = LCH_ListCreate();
  size_t to, from = 0, len = strlen(str);
  bool is_delim, was_delim = true;

  for (to = 0; to < len; to++) {
    is_delim = strchr(del, str[to]) != NULL;
    if (is_delim) {
      if (was_delim) {
        continue;
      }
      assert(to > from);
      char *s = strndup(str + from, to - from);
      if (s == NULL) {
        LCH_LOG_ERROR("Failed to allocate memory during string split: %s",
                      strerror(errno));
        LCH_ListDestroy(list);
        return NULL;
      }
      if (!LCH_ListAppend(list, (void *)s, free)) {
        free(s);
        LCH_ListDestroy(list);
        return NULL;
      }
    } else {
      if (was_delim) {
        from = to;
      }
    }
    was_delim = is_delim;
  }

  if (from < to && !is_delim) {
    char *s = strndup(str + from, to - from);
    if (s == NULL) {
      LCH_ListDestroy(list);
      return NULL;
    }
    if (!LCH_ListAppend(list, (void *)s, free)) {
      LCH_ListDestroy(list);
      return NULL;
    }
  }

  return list;
}

bool LCH_StringStartsWith(const char *const self, const char *const substr) {
  assert(self != NULL);
  assert(substr != NULL);

  size_t length = strlen(substr);
  for (size_t i = 0; i < length; i++) {
    if (self[i] != substr[i]) {
      return false;
    }
  }
  return true;
}

char *LCH_StringStrip(char *str, const char *charset) {
  assert(str != NULL);

  size_t start = 0, end = 0, cursor = 0;
  while (str[cursor] != '\0') {
    if (strchr(charset, str[cursor]) != NULL) {
      if (start == cursor) {
        ++start;
      }
    } else {
      end = cursor + 1;
    }
    ++cursor;
  }

  str = (char *)memmove((void *)str, (void *)(str + start), end - start);
  str[end - start] = '\0';
  return str;
}

bool LCH_FileSize(FILE *file, size_t *size) {
  if (fseek(file, 0, SEEK_END) != 0) {
    LCH_LOG_ERROR("Failed to seek to end of file: %s", strerror(errno));
    return false;
  }

  long pos = ftell(file);
  if (pos < 0) {
    LCH_LOG_ERROR("Failed to obtain the current file position indicator: %s",
                  strerror(errno));
    return false;
  }
  *size = (size_t)pos;

  if (fseek(file, 0, SEEK_SET) != 0) {
    LCH_LOG_ERROR("Failed to seek to start of file: %s", strerror(errno));
    return false;
  }

  return true;
}

unsigned char *LCH_SHA1(const void *const message, const size_t length) {
  EVP_MD_CTX *context = EVP_MD_CTX_new();
  if (context == NULL) {
    LCH_LOG_ERROR("Failed to create context for digest operation: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    return NULL;
  }

  EVP_MD *sha1 = EVP_MD_fetch(NULL, "SHA1", NULL);
  if (sha1 == NULL) {
    LCH_LOG_ERROR("Failed to fetch SHA1 algorithm for digest operation: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    EVP_MD_CTX_free(context);
  }

  if (!EVP_DigestInit_ex(context, sha1, NULL)) {
    LCH_LOG_ERROR("Failed to initialize digest operation: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    EVP_MD_CTX_free(context);
    EVP_MD_free(sha1);
    return NULL;
  }

  if (!EVP_DigestUpdate(context, message, length)) {
    LCH_LOG_ERROR("Failed to pass message to be digested: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    EVP_MD_CTX_free(context);
    EVP_MD_free(sha1);
    return NULL;
  }

  unsigned char *const digest = OPENSSL_malloc(EVP_MD_get_size(sha1));
  if (digest == NULL) {
    LCH_LOG_ERROR("Failed to allocate memory for digest buffer: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    EVP_MD_CTX_free(context);
    EVP_MD_free(sha1);
    return NULL;
  }

  if (!EVP_DigestFinal_ex(context, digest, NULL)) {
    LCH_LOG_ERROR("Failed to calculate digest: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    EVP_MD_CTX_free(context);
    EVP_MD_free(sha1);
    return NULL;
  }

  EVP_MD_CTX_free(context);
  EVP_MD_free(sha1);
  return digest;
}
