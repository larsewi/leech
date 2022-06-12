#include <assert.h>
#include <ctype.h>
#include <leech.h>
#include <leech_csv.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LENGTH(x) (sizeof(x) / sizeof(*x))

static char *BOOTSTRAP_ADDRESS = "127.0.0.1";
static bool LOG_DEBUG = false;
static bool LOG_VERBOSE = false;

static void CheckOpts(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  int rc = EXIT_FAILURE;
  LCH_Instance *instance = NULL;

  CheckOpts(argc, argv);

  { // Create instance
    LCH_InstanceCreateInfo createInfo = {
        .instanceID = BOOTSTRAP_ADDRESS,
        .workDir = ".leech/",
    };
    if ((instance = LCH_InstanceCreate(&createInfo)) == NULL) {
      fprintf(stderr, "LCH_InstanceCreate\n");
      goto exit_failure;
    }
  }

  { // Add debug messenger
    LCH_DebugMessengerCreateInfo createInfo = {
        .severity = LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT |
                    LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT |
                    LCH_DEBUG_MESSAGE_TYPE_INFO_BIT,
        .messageCallback = &LCH_DebugMessengerCallbackDefault,
    };
    if (LOG_VERBOSE) {
      createInfo.severity |= LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT;
    }
    if (LOG_DEBUG) {
      createInfo.severity |= LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT;
    }
    if (!LCH_DebugMessengerAdd(instance, &createInfo)) {
      fprintf(stderr, "LCH_DebugMessengerAdd\n");
      goto exit_failure;
    }
  }

  { // Add CSV table
    LCH_TableCreateInfo createInfo = {
        .locator = "example.csv",
        .readCallback = LCH_TableReadCallbackCSV,
        .writeCallback = LCH_TableWriteCallbackCSV,
    };
    if (!LCH_TableAdd(instance, &createInfo)) {
      fprintf(stderr, "LCH_TableAdd\n");
      goto exit_failure;
    }

    // Remove:
    char ****table = NULL;
    if (!createInfo.readCallback(instance, createInfo.locator, table)) {
      printf("Failed\n");
      goto exit_failure;
    }
  }

  rc = EXIT_SUCCESS;
exit_failure:
  LCH_InstanceDestroy(instance);
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
      // TODO: print help message
      exit(EXIT_SUCCESS);
      break;
    default:
      exit(EXIT_FAILURE);
      break;
    }
  }
}
