#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "debug_messenger.h"

/* The grammar appears as follows:
 * file = header *(CRLF record) [CRLF]
 * header = name *(COMMA name)
 * record = field *(COMMA field)
 * name = field
 * field = (escaped / non-escaped)
 * escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE
 * non-escaped = *TEXTDATA
 * COMMA = %x2C
 * CR = %x0D
 * DQUOTE =  %x22
 * LF = %x0A
 * CRLF = CR LF
 * TEXTDATA =  %x20-21 / %x23-2B / %x2D-7E
 */

#define TEXTDATA(ch) ((ch >= ' ' && ch <= '!') || (ch >= '#' && ch <= '+') || ch >= '-' && ch <= '~')
#define CR(ch) (ch == '\r')
#define LF(ch) (ch == '\n')
#define DQUOTE(ch) (ch == '"')
#define COMMA(ch) (ch == ',')

static char *NonEscaped(const char *const buffer, const size_t size,
                        size_t *at) {
  const size_t from = *at;
  size_t to = from;

  char ch = buffer[to];
  while (to < size && TEXTDATA(ch)) {
    ch = buffer[++to];
  }

  if (from == to) {
    return NULL;
  }

  char *term = strndup(buffer + from, to - from);
  *at = to;
  return term;
}

// static char *Escaped(const char *const buffer, const size_t size, size_t *at) {
//   const size_t from = *at;
//   size_t to = from;

//   if (!DQUOTE(buffer[to])) {
//     return NULL;
//   }
//   to += 1;

//   while ((to < size && (TEXTDATA(buffer, to) || COMMA(buffer, to) || CR(buffer, to) || LF(buffer, to)) || (to < size + 1 && DQUOTE(buffer, to) && DQUOTE(buffer, to + 1)))) {
//     to += 1;
//   }

//   char *term = strndup(buffer + from, to - from);
//   *at = to;
//   return term;
// }
