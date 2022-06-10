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
static LCH_DebugMessenger *CreateDebugMessenger(void);
static LCH_Instance *CreateInstance(LCH_DebugMessenger *debugMessenger);

int main(int argc, char *argv[]) {
  int rc = EXIT_FAILURE;
  LCH_DebugMessenger *debugMessenger = NULL;
  LCH_Instance *instance = NULL;

  CheckOpts(argc, argv);

  if ((debugMessenger = CreateDebugMessenger()) == NULL) {
    goto exit_failure;
  }

  if ((instance = CreateInstance(debugMessenger)) == NULL) {
    goto exit_failure;
  }

  LCH_TestFunc(instance);

  rc = EXIT_SUCCESS;
exit_failure:
  LCH_InstanceDestroy(&instance);
  LCH_DebugMessengerDestroy(&debugMessenger);
  return rc;
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

static LCH_DebugMessenger *CreateDebugMessenger(void) {
  LCH_DebugMessengerCreateInfo createInfo = {0};
  createInfo.severity = LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT |
                        LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT |
                        LCH_DEBUG_MESSAGE_TYPE_INFO_BIT;
  if (LOG_VERBOSE) {
    createInfo.severity |= LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT;
  }
  if (LOG_DEBUG) {
    createInfo.severity |= LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT;
  }
  createInfo.callback = LCH_DebugMessengerCallback;
  return LCH_DebugMessengerCreate(&createInfo);
}

static LCH_Instance *CreateInstance(LCH_DebugMessenger *debugMessenger) {
  LCH_InstanceCreateInfo createInfo = {0};
  createInfo.instanceID = BOOTSTRAP_ADDRESS;
  createInfo.debugMessenger = debugMessenger;
  return LCH_InstanceCreate(&createInfo);
}
