#include <assert.h>
#include <config.h>
#include <errno.h>
#include <leech.h>
#include <leech_csv.h>
#include <leech_psql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "commit.h"
#include "diff.h"
#include "patch.h"

#define WORK_DIR ".leech/"

enum OPTION_VALUE {
  OPTION_FILE = 1,
  OPTION_INFO,
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
  { "info", no_argument, NULL, OPTION_INFO },
  { "verbose", no_argument, NULL, OPTION_VERBOSE },
  { "debug", no_argument, NULL, OPTION_DEBUG },
  { "version", no_argument, NULL, OPTION_VERSION },
  { "help", no_argument, NULL, OPTION_HELP },
  { NULL, 0, NULL, 0 },
};

static const char *const DESCRIPTIONS[] = {
  "enable debug messages",
  "enable verbose messages",
  "enable info messages",
  "print version string",
  "print help message",
};

static const struct command COMMANDS[] = {
  {"commit", "commit changes in tables", Commit},
  {"diff", "calculate changes in tables", Diff},
  {"patch", "apply changes to tables", Patch},
  {NULL, NULL, NULL},
};

static void PrintVersion(void) {
  printf("%s\n", PACKAGE_STRING);
}

static void PrintOptions(void) {
  size_t longest = 0;
  for (int i = 0; OPTIONS[i].val != 0; i++) {
    const size_t length = strlen(OPTIONS[i].name);
    longest = (length > longest) ? length : longest;
  }

  char format[512];
  int ret = snprintf(format, sizeof(format), "  --%%-%zus  %%s\n", longest);
  assert(ret >= 0 && sizeof(format) > (size_t)ret);

  printf("options:\n");
  for (int i = 0; OPTIONS[i].val != 0; i++) {
    printf(format, OPTIONS[i].name, DESCRIPTIONS[i]);
  }
}

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
  PrintOptions();
  printf("\n");
  PrintCommands();
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
  unsigned char severity = LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT | LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT;

  int opt;
  while ((opt = getopt_long(argc, argv, "+", OPTIONS, NULL)) != -1) {
    switch (opt) {
      case OPTION_INFO:
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

  for (int i = 0; COMMANDS[i].name != NULL; i++) {
    if (strcmp(argv[optind], COMMANDS[i].name) == 0) {
      return COMMANDS[i].command(argc - optind, argv + optind);
    }
  }
  return EXIT_FAILURE;
}



// static LCH_Instance *SetupInstance(void) {
//   LCH_Instance *instance = NULL;
//   {  // Create instance
//     LCH_InstanceCreateInfo createInfo = {
//         .instanceID = UNIQUE_ID,
//         .workDir = WORK_DIR,
//     };

//     instance = LCH_InstanceCreate(&createInfo);
//     if (instance == NULL) {
//       LCH_LOG_ERROR("LCH_InstanceCreate");
//       return NULL;
//     }
//   }

//   {  // Add CSV table
//     LCH_TableCreateInfo createInfo = {
//         .readLocator = READ_LOCATOR,
//         .readCallback = LCH_TableReadCallbackCSV,
//         .writeLocator = WRITE_LOCATOR,
//         .writeCallback = LCH_TableWriteCallbackCSV,
//     };

//     LCH_Table *table = LCH_TableCreate(&createInfo);
//     if (table == NULL) {
//       LCH_LOG_ERROR("LCH_TableCreate");
//       return NULL;
//     }

//     // TODO: Add table to instance
//     LCH_TableDestroy(table);
//   }

//   return instance;
// }
