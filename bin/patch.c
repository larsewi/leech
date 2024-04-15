#include "patch.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lib/utils.h"
#include "common.h"

enum OPTION_VALUE {
  OPTION_FIELD = 1,
  OPTION_VALUE,
  OPTION_FILE,
  OPTION_HELP,
};

static const struct option OPTIONS[] = {
    {"field", required_argument, NULL, OPTION_FIELD},
    {"value", required_argument, NULL, OPTION_VALUE},
    {"file", required_argument, NULL, OPTION_FILE},
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
    "field name of source identifiers",
    "unique identifier of source",
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

int Patch(const char *const work_dir, int argc, char *argv[]) {
  assert(work_dir != NULL);

  const char *patch_file = NULL;
  const char *uid_field = NULL;
  const char *uid_value = NULL;

  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_FIELD:
        uid_field = optarg;
        break;
      case OPTION_VALUE:
        uid_value = optarg;
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

  if (uid_field == NULL) {
    LCH_LOG_ERROR("Missing required argument --field ...");
    return EXIT_FAILURE;
  }

  if (uid_value == NULL) {
    LCH_LOG_ERROR("Missing required argument --value ...");
    return EXIT_FAILURE;
  }

  if (patch_file == NULL) {
    LCH_LOG_ERROR("Missing required argument --file ...");
    return EXIT_FAILURE;
  }

  LCH_Buffer *const buffer = LCH_BufferCreate();
  if (buffer == NULL) {
    return EXIT_FAILURE;
  }

  if (!LCH_BufferReadFile(buffer, patch_file)) {
    LCH_BufferDestroy(buffer);
    return EXIT_FAILURE;
  }

  const char *const data = LCH_BufferData(buffer);
  const size_t size = LCH_BufferLength(buffer);
  LCH_LOG_DEBUG("Loaded patch file '%s' %zu Bytes.", patch_file, size);

  if (!LCH_Patch(work_dir, uid_field, uid_value, data, size)) {
    LCH_LOG_ERROR("Failed to apply patch from file '%s'.", patch_file);
    LCH_BufferDestroy(buffer);
    return EXIT_FAILURE;
  }

  LCH_BufferDestroy(buffer);
  return EXIT_SUCCESS;
}
