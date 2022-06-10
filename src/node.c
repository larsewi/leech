#include <ctype.h>
#include <leech.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *BOOTSTRAP_ADDRESS = NULL;
static bool LOG_DEBUG = false;
static bool LOG_VERBOSE = false;

static void CheckOpts(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  CheckOpts(argc, argv);

  LCH_DebugMessenger debugMessenger = {0};
  debugMessenger.severity = LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT |
                            LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT |
                            LCH_DEBUG_MESSAGE_TYPE_INFO_BIT;
  if (LOG_VERBOSE) {
    debugMessenger.severity |= LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT;
  }
  if (LOG_DEBUG) {
    debugMessenger.severity |= LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT;
  }
  debugMessenger.callback = LCH_DebugMessengerCallback;

  LCH_Instance instance = {0};
  instance.debugMessenger = &debugMessenger;

  LCH_TestFunc(&instance);

  return 0;
}

static void CheckOpts(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "b:vdh")) != -1) {
    switch (opt) {
    case 'b':
      BOOTSTRAP_ADDRESS = optarg;
      break;

    case 'd':
      LOG_DEBUG = true;
      break;

    case 'v':
      LOG_VERBOSE = true;
      break;

    case 'h':
      printf("%s: [OPTION]...\n", argv[0]);
      exit(EXIT_SUCCESS);
      break;

    default:
      fprintf(stderr, "Bad option '%c'\n", (char)opt);
      exit(EXIT_FAILURE);
      break;
    }
  }
}
