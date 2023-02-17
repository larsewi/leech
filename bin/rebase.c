#include "rebase.h"

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

int Rebase(const char *const work_dir, int argc, char *argv[]) {
  assert(work_dir != NULL);

  (void)work_dir; /* TODO: Create instance and remove */

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
  LCH_LOG_WARNING("Not implemented ...");
  return EXIT_SUCCESS;
}
