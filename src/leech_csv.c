/**
 * CSV Standard rfc4180
 */

#include <stdio.h>
#include <stdlib.h>

#include "leech_csv.h"

bool LCH_TableReadCallbackCSV(const char *filename, char ****table) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("fopen");
    return false;
  }

  if (fseek(file, 0, SEEK_END) < 0) {
    perror("fseek");
    return false;
  }

  const long size = ftell(file);
  if (size < 0) {
    perror("ftell");
    return false;
  }

  if (fseek(file, 0, SEEK_SET) < 0) {
    perror("fseek");
    return false;
  }

  char *buffer = malloc(size + 1);
  if (buffer == NULL) {
    perror("malloc");
    return NULL;
  }
  buffer[size] = '\0';

  if (fread(buffer, 1, size, file) != size) {
    perror("fread");
    return false;
  }

  long l = 0;
  long r = size;

  printf("%s", buffer);
  return true;
}

bool LCH_TableWriteCallbackCSV(const char *filename, char ****table) {}

static void t_TEXTDATA(void) {

}

static void t_CRLF(void) {

}

static void t_LF(void) {

}

static void t_DQUOTE(void) {

}

static void t_CR(void) {

}

static void t_COMMA(void) {

}

static void p_non_escaped(void) {
  // non_escaped = *TEXTDATA
}

static void p_escaped(void) {
  // escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE
}

static void p_field(void) {
  // field = (escaped / non-escaped)
}

static void p_name(void) {
  // name = field
}

static void p_record(void) {
  // record = field *(COMMA field)
}

static void p_header(void) {
  // header = name *(COMMA name)
}

static void p_file(void) {
  // file = [header CRLF] record * (CRLF record) [CRLF]
}
