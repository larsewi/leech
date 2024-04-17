#include "history.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../lib/csv.h"
#include "common.h"

enum OPTION_VALUE {
  OPTION_PRIMARY = 1,
  OPTION_FROM,
  OPTION_TO,
  OPTION_FILE,
  OPTION_HELP,
};

struct arguments {
  const char *arg;
  const char *desc;
};

static const struct option OPTIONS[] = {
    {"primary", required_argument, NULL, OPTION_PRIMARY},
    {"from", optional_argument, NULL, OPTION_FROM},
    {"to", optional_argument, NULL, OPTION_TO},
    {"file", required_argument, NULL, OPTION_FILE},
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
    "primary fields",
    "timestamp from (default 0)",
    "timestamp to (default now)",
    "output history file",
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

int History(const char *const work_dir, int argc, char *argv[]) {
  assert(work_dir != NULL);

  const char *primary = NULL;
  const char *filename = NULL;
  double from = 0.0;
  double to = (double)time(NULL);

  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_PRIMARY:
        primary = optarg;
        break;
      case OPTION_FROM:
        if (sscanf(optarg, "%lf", &from) != 1) {
          fprintf(stderr, "Failed to parse timestamp in option --from: %s",
                  strerror(errno));
          return EXIT_FAILURE;
        }
        break;
      case OPTION_TO:
        if (sscanf(optarg, "%lf", &to) != 1) {
          fprintf(stderr, "Failed to parse timestamp in option --to: %s",
                  strerror(errno));
          return EXIT_FAILURE;
        }
        break;
      case OPTION_FILE:
        filename = optarg;
        break;
      case OPTION_HELP:
        PrintHelp();
        return EXIT_SUCCESS;
      default:
        return EXIT_FAILURE;
    }
  }

  if (primary == NULL) {
    fprintf(stderr, "Missing required argument --primary\n");
    return EXIT_SUCCESS;
  }

  if (filename == NULL) {
    fprintf(stderr, "Missing required argument --file\n");
    return EXIT_SUCCESS;
  }

  LCH_List *const primary_fields = LCH_CSVParseRecord(primary, strlen(primary));
  if (primary_fields == NULL) {
    return EXIT_FAILURE;
  }

  LCH_Buffer *const history = LCH_History(work_dir, primary_fields, from, to);
  if (history == NULL) {
    fprintf(stderr, "LCH_History\n");
    LCH_ListDestroy(primary_fields);
    return EXIT_FAILURE;
  }

  if (!LCH_BufferWriteFile(history, filename)) {
    LCH_BufferDestroy(history);
    return EXIT_FAILURE;
  }

  LCH_BufferDestroy(history);
  return EXIT_SUCCESS;
}
