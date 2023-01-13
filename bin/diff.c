#include "diff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"

enum OPTION_VALUE {
  OPTION_FILE = 1,
  OPTION_HELP,
};

struct arguments {
  const char *arg;
  const char *desc;
};

static const struct option OPTIONS[] = {
    {"file", required_argument, NULL, OPTION_FILE},
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
    "output patch file",
    "print help message",
};

static const struct arguments ARGUMENTS[] = {
    { "block", "identifier (block hash)" },
    { NULL, NULL },
};

static void PrintArguments(void) {
  size_t longest = 0;
  for (int i = 0; ARGUMENTS[i].arg != NULL; i++) {
    const size_t length = strlen(ARGUMENTS[i].arg);
    longest = (length > longest) ? length : longest;
  }

  char format[512];
  int ret = snprintf(format, sizeof(format), "  %%-%zus  %%s\n", longest);
  assert(ret >= 0 && sizeof(format) > (size_t)ret);

  printf("arguments:\n");
  for (int i = 0; ARGUMENTS[i].arg != NULL; i++) {
    printf(format, ARGUMENTS[i].arg, ARGUMENTS[i].desc);
  }
}

static void PrintHelp(void) {
  PrintVersion();
  printf("\n");
  PrintArguments();
  printf("\n");
  PrintOptions(OPTIONS, DESCRIPTIONS);
  printf("\n");
  PrintBugreport();
  printf("\n");
}

int Diff(int argc, char *argv[]) {
  const char *patch_file = NULL;
  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_FILE:
        patch_file = optarg;
        break;
      case OPTION_HELP:
        PrintHelp();
        return EXIT_SUCCESS;
      default:
        return EXIT_FAILURE;
    }
  }

  if (optind >= argc) {
    LCH_LOG_ERROR("Missing required argument ...");
    return EXIT_FAILURE;
  }
  const char *const block_id = argv[optind];

  LCH_Instance *instance = SetupInstance();
  if (instance == NULL) {
    LCH_LOG_ERROR("SetupInstance");
    return EXIT_FAILURE;
  }

  char *diff = LCH_InstanceDiff(instance, block_id, patch_file);
  if (diff == NULL) {
    LCH_LOG_ERROR("Failed to enumerate blocks.");
    LCH_InstanceDestroy(instance);
    return EXIT_FAILURE;
  }

  LCH_InstanceDestroy(instance);

  return EXIT_SUCCESS;
}
