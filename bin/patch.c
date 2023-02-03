#include "patch.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

enum OPTION_VALUE {
  OPTION_FILE = 1,
  OPTION_HELP,
};

static const struct option OPTIONS[] = {
    {"file", required_argument, NULL, OPTION_FILE},
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
    "input patch file",
    "print help message",
};

static void PrintHelp(void) {
  PrintVersion();
  printf("\n");
  PrintOptions(OPTIONS, DESCRIPTIONS);
  printf("\n");
  PrintBugreport();
  printf("\n");
}

int Patch(int argc, char *argv[]) {
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

  if (patch_file == NULL) {
    LCH_LOG_WARNING("No patch file selected ...");
    return EXIT_SUCCESS;
  }

  return EXIT_SUCCESS;
}
