#include "lch_utils.h"

/**
 * Written by Daniel J. Bernstein (also known as djb), this simple hash function
 * dates back to 1991. */
unsigned long LCH_Hash(unsigned char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++) != 0) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}
