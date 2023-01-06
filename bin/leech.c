#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <../lib/leech.h>
#include <../lib/leech_csv.h>
#include <../lib/leech_psql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commit.h"
#include "common.h"
#include "diff.h"
#include "patch.h"

#define WORK_DIR ".leech/"

enum OPTION_VALUE {
  OPTION_FILE = 1,
  OPTION_INFORM,
  OPTION_VERBOSE,
  OPTION_DEBUG,
  OPTION_VERSION,
  OPTION_HELP,
};

struct command {
  const char *name;
  const char *desc;
  int (*command)(int, char *[]);
};

static const struct option OPTIONS[] = {
    {"inform", no_argument, NULL, OPTION_INFORM},
    {"verbose", no_argument, NULL, OPTION_VERBOSE},
    {"debug", no_argument, NULL, OPTION_DEBUG},
    {"version", no_argument, NULL, OPTION_VERSION},
    {"help", no_argument, NULL, OPTION_HELP},
    {NULL, 0, NULL, 0},
};

static const char *const DESCRIPTIONS[] = {
    "enable debug messages", "enable verbose messages", "enable info messages",
    "print version string",  "print help message",
};

static const struct command COMMANDS[] = {
    {"commit", "commit changes in tables", Commit},
    {"diff", "calculate changes in tables", Diff},
    {"patch", "apply changes to tables", Patch},
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
  PrintOptions(OPTIONS, DESCRIPTIONS);
  printf("\n");
  PrintCommands();
  printf("\n");
  PrintBugreport();
  printf("\n");
}

static void SetupDebugMessenger(unsigned char severity) {
  LCH_DebugMessengerInitInfo initInfo = {
      .severity = severity,
      .messageCallback = &LCH_DebugMessengerCallbackDefault,
  };
  LCH_DebugMessengerInit(&initInfo);
}

int main(int argc, char *argv[]) {
  unsigned char severity =
      LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT | LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT;

  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_INFORM:
        severity |= LCH_DEBUG_MESSAGE_TYPE_INFO_BIT;
        break;
      case OPTION_VERBOSE:
        severity |= LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT;
        break;
      case OPTION_DEBUG:
        severity |= LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT;
        break;
      case OPTION_VERSION:
        PrintVersion();
        return EXIT_SUCCESS;
      case OPTION_HELP:
        PrintHelp();
        return EXIT_SUCCESS;
      default:
        return EXIT_FAILURE;
    }
  }

  SetupDebugMessenger(severity);

  if (optind >= argc) {
    LCH_LOG_WARNING("NOOP: Missing command ...");
    return EXIT_SUCCESS;
  }

  for (int i = 0; COMMANDS[i].name != NULL; i++) {
    if (strcmp(argv[optind], COMMANDS[i].name) == 0) {
      optind += 1;
      return COMMANDS[i].command(argc, argv);
    }
  }
  return EXIT_FAILURE;
}
