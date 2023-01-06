#include "commit.h"

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

int Commit(int argc, char *argv[]) {
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

  LCH_Instance *instance = SetupInstance();
  if (instance == NULL) {
    LCH_LOG_ERROR("SetupInstance");
    return EXIT_FAILURE;
  }

  if (!LCH_InstanceCommit(instance)) {
    LCH_LOG_ERROR("LCH_InstanceCommit");
    LCH_InstanceDestroy(instance);
    return EXIT_FAILURE;
  }

  LCH_InstanceDestroy(instance);

  return EXIT_SUCCESS;
}
