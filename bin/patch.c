#include "patch.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../lib/utils.h"
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

int Patch(const char *const unique_id, const char *const work_dir, int argc, char *argv[]) {
  assert(unique_id != NULL);
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
    LCH_LOG_WARNING("No patch file selected ...");
    return EXIT_SUCCESS;
  }

  size_t size;
  char *const buffer = LCH_ReadFile(patch_file, &size);
  if (buffer == NULL) {
    LCH_LOG_ERROR("Failed to load patch file '%s'.", patch_file);
    return EXIT_FAILURE;
  }
  LCH_LOG_DEBUG("Loaded patch file '%s' %zu Bytes.", patch_file, size);

  LCH_Instance *const instance = SetupInstance(unique_id, work_dir);
  if (instance == NULL) {
    LCH_LOG_ERROR("Failed to setup leech instance.");
    free(buffer);
    return EXIT_FAILURE;
  }

  if (!LCH_InstancePatch(instance, buffer, size)) {
    LCH_LOG_ERROR("Failed to apply patch from file '%s'.", patch_file);
    LCH_InstanceDestroy(instance);
    free(buffer);
    return EXIT_FAILURE;
  }

  LCH_InstanceDestroy(instance);
  free(buffer);
  return EXIT_SUCCESS;
}
