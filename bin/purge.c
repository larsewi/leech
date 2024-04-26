#include "purge.h"

#include <stdio.h>

#include "common.h"

enum OPTION_VALUE {
  OPTION_HELP = 1,
};

struct arguments {
  const char *arg;
  const char *desc;
};

static const struct option OPTIONS[] = {
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
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

int Purge(const char *const work_dir, int argc, char *argv[]) {
  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_HELP:
        PrintHelp();
        return EXIT_SUCCESS;
      default:
        return EXIT_FAILURE;
    }
  }

  if (!LCH_Purge(work_dir)) {
    fprintf(stderr, "Failed to purge blocks");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
