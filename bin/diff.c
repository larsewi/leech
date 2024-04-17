#include "diff.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

enum OPTION_VALUE {
  OPTION_BLOCK = 1,
  OPTION_FILE,
  OPTION_HELP,
};

struct arguments {
  const char *arg;
  const char *desc;
};

static const struct option OPTIONS[] = {
    {"block", required_argument, NULL, OPTION_BLOCK},
    {"file", required_argument, NULL, OPTION_FILE},
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
    "last seen block",
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

int Diff(const char *const work_dir, int argc, char *argv[]) {
  assert(work_dir != NULL);
  const char *patch_file = NULL;
  const char *block_id = "0000000000000000000000000000000000000000";

  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_BLOCK:
        block_id = optarg;
        break;
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

  if (block_id == NULL) {
    fprintf(stderr, "Missing required argument --block\n");
    return EXIT_SUCCESS;
  }

  if (patch_file == NULL) {
    fprintf(stderr, "Missing required argument --file\n");
    return EXIT_SUCCESS;
  }

  LCH_Buffer *patch = LCH_Diff(work_dir, block_id);
  if (patch == NULL) {
    fprintf(stderr, "LCH_Diff\n");
    return EXIT_FAILURE;
  }

  if (!LCH_BufferWriteFile(patch, patch_file)) {
    LCH_BufferDestroy(patch);
    return EXIT_FAILURE;
  }

  LCH_BufferDestroy(patch);
  return EXIT_SUCCESS;
}
