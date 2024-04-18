#include "rebase.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    "output patch file",
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
    fprintf(stderr, "Missing required argument --file\n");
    return EXIT_FAILURE;
  }

  LCH_Buffer *const patch = LCH_Rebase(work_dir);
  if (patch == NULL) {
    fprintf(stderr, "LCH_Rebase\n");
    return EXIT_FAILURE;
  }

  if (!LCH_BufferWriteFile(patch, patch_file)) {
    LCH_BufferDestroy(patch);
    return EXIT_FAILURE;
  }

  LCH_BufferDestroy(patch);
  return EXIT_SUCCESS;
}
