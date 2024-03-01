#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void PrintVersion(void) { printf("%s\n", PACKAGE_STRING); }

void PrintOptions(const struct option *const options,
                  const char *const *const descriptions) {
  size_t longest = 0;
  for (int i = 0; options[i].val != 0; i++) {
    const size_t length = strlen(options[i].name);
    longest = (length > longest) ? length : longest;
  }

  char format[512];
  int ret = snprintf(format, sizeof(format), "  --%%-%zus  %%s\n", longest);
  assert(ret >= 0 && sizeof(format) > (size_t)ret);

  printf("options:\n");
  for (int i = 0; options[i].val != 0; i++) {
    printf(format, options[i].name, descriptions[i]);
  }
}

void PrintBugreport(void) {
  printf("Report bugs to <%s>.\n", PACKAGE_BUGREPORT);
  printf("%s home page: <%s>.\n", PACKAGE_NAME, PACKAGE_URL);
}
