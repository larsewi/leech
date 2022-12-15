#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <string.h>
#include <assert.h>

void PrintVersion(void) {
  printf("%s\n", PACKAGE_STRING);
}

void PrintOptions(const struct option *const options, const char *const *const descriptions) {
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
