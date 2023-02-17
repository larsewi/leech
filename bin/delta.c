#include "delta.h"

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

int Delta(const char *const work_dir, int argc, char *argv[]) {
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

  LCH_Instance *instance = SetupInstance(work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("SetupInstance");
    return EXIT_FAILURE;
  }

  size_t size;
  char *diff = LCH_InstanceDelta(instance, block_id, &size);
  if (diff == NULL) {
    LCH_LOG_ERROR("Failed to enumerate blocks.");
    LCH_InstanceDestroy(instance);
    return EXIT_FAILURE;
  }

  LCH_InstanceDestroy(instance);

  if (patch_file == NULL) {
    free(diff);
    return EXIT_SUCCESS;
  }

  FILE *file = fopen(patch_file, "wb");
  if (file == NULL) {
    LCH_LOG_ERROR("Failed to open file '%s' for binary writing: %s",
                  strerror(errno));
    free(diff);
    return EXIT_FAILURE;
  }

  if (fwrite(diff, 1, size, file) != size) {
    LCH_LOG_ERROR("Failed to write to file '%s'.", strerror(errno));
    fclose(file);
    free(diff);
    return EXIT_FAILURE;
  }

  fclose(file);
  free(diff);
  return EXIT_SUCCESS;
}
