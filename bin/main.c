#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../lib/leech.h"
#include "commit.h"
#include "common.h"
#include "diff.h"
#include "history.h"
#include "patch.h"
#include "rebase.h"

#ifdef HAVE_LIBPQ
#include "libpq-fe.h"
#endif  // HAVE_LIBPQ

enum OPTION_VALUE {
  OPTION_WORKDIR = 1,
  OPTION_INFORM,
  OPTION_VERBOSE,
  OPTION_DEBUG,
  OPTION_VERSION,
  OPTION_HELP,
};

struct command {
  const char *name;
  const char *desc;
  int (*command)(const char *, int, char *[]);
};

static const struct option OPTIONS[] = {
    {"workdir", required_argument, NULL, OPTION_WORKDIR},
    {"inform", no_argument, NULL, OPTION_INFORM},
    {"verbose", no_argument, NULL, OPTION_VERBOSE},
    {"debug", no_argument, NULL, OPTION_DEBUG},
    {"version", no_argument, NULL, OPTION_VERSION},
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
    "set work directory",    "enable info messages", "enable verbose messages",
    "enable debug messages", "print version string", "print help message",
};

static const struct command COMMANDS[] = {
    {"commit", "compute and commit changes in tables", Commit},
    {"diff", "merge changes in tables", Diff},
    {"rebase", "rebase to current table state", Rebase},
    {"patch", "apply changes to tables", Patch},
    {"history", "get history of a specific record", History},
    {NULL, NULL, NULL},
};

static void PrintCommands(void) {
  size_t longest = 0;
  for (int i = 0; COMMANDS[i].name != NULL; i++) {
    const size_t length = strlen(COMMANDS[i].name);
    longest = (length > longest) ? length : longest;
  }

  char format[512];
  int ret = snprintf(format, sizeof(format), "  %%-%zus  %%s\n", longest);
  assert(ret >= 0 && sizeof(format) > (size_t)ret);

  printf("commands:\n");
  for (int i = 0; COMMANDS[i].name != NULL; i++) {
    printf(format, COMMANDS[i].name, COMMANDS[i].desc);
  }
}

static void PrintHelp(void) {
  PrintVersion();
  printf("\n");
  PrintCommands();
  printf("\n");
  PrintOptions(OPTIONS, DESCRIPTIONS);
  printf("\n");
  PrintBugreport();
  printf("\n");
}

int main(int argc, char *argv[]) {
  unsigned char severity =
      LCH_LOGGER_MESSAGE_TYPE_ERROR_BIT | LCH_LOGGER_MESSAGE_TYPE_WARNING_BIT;

  const char *work_dir = ".leech";

  /**
   * For some reason, if we don't reference something from libpq, we get
   * "undefined reference" errors when dlopen'ing the leech_psql.so, which uses
   * libpq. My wildest guess is that libpq is stripped away if the build system
   * doesn't find any references to it. Adding a reference to one of the symbols
   * here, seems to fix the issue.
   */
#ifdef HAVE_LIBPQ
  PQlibVersion();
#endif  // HAVE_LIBPQ

  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_WORKDIR:
        work_dir = optarg;
        break;
      case OPTION_DEBUG:
        severity |= LCH_LOGGER_MESSAGE_TYPE_DEBUG_BIT;
        // fallthrough
      case OPTION_VERBOSE:
        severity |= LCH_LOGGER_MESSAGE_TYPE_VERBOSE_BIT;
        // fallthrough
      case OPTION_INFORM:
        severity |= LCH_LOGGER_MESSAGE_TYPE_INFO_BIT;
        break;
      case OPTION_VERSION:
        PrintVersion();
        return EXIT_SUCCESS;
      case OPTION_HELP:
        PrintHelp();
        return EXIT_SUCCESS;
      default:
        fprintf(stderr, "Illegal option: '%s'\n", optarg);
        return EXIT_FAILURE;
    }
  }

  LCH_LoggerSeveritySet(severity);

  if (optind >= argc) {
    fprintf(stderr, "Missing command ...");
    return EXIT_SUCCESS;
  }

  for (int i = 0; COMMANDS[i].name != NULL; i++) {
    if (strcmp(argv[optind], COMMANDS[i].name) == 0) {
      optind += 1;
      return COMMANDS[i].command(work_dir, argc, argv);
    }
  }
  return EXIT_FAILURE;
}
