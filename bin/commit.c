#include "commit.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

enum OPTION_VALUE {
  OPTION_HELP = 1,
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

int Commit(const char *const work_dir, int argc, char *argv[]) {
  assert(work_dir != NULL);

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

  if (!LCH_Commit(work_dir)) {
    LCH_LOG_ERROR("LCH_Commit");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
