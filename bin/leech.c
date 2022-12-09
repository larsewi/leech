#include <assert.h>
#include <config.h>
#include <errno.h>
#include <leech_csv.h>
#include <leech_psql.h>
#include <leech.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WORK_DIR ".leech/"
#define MAX_EVENTS 10

#define OPTIONS \
    "-D  enable debug messages\n" \
    "-V  enable verbose messages\n" \
    "-v  print version number\n" \
    "-h  print help message\n"

static const char *READ_LOCATOR = NULL;
static const char *WRITE_LOCATOR = NULL;
static const char *UNIQUE_ID = NULL;
static bool LOG_DEBUG = false;
static bool LOG_VERBOSE = false;

static void CheckOptions(int argc, char *argv[]);
static void SetupDebugMessenger(void);
static LCH_Instance *SetupInstance(void);

int main(int argc, char *argv[]) {
  CheckOptions(argc, argv);

  SetupDebugMessenger();

  LCH_Instance *instance = SetupInstance();
  if (instance == NULL) {
    return EXIT_FAILURE;
  }

  LCH_InstanceDestroy(instance);
  return EXIT_SUCCESS;
}

static void CheckOptions(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "urwDVvh")) != -1) {
    switch (opt) {
      case 'u':
        UNIQUE_ID = optarg;
        break;
      case 'r':
        READ_LOCATOR = optarg;
        break;
      case 'w':
        WRITE_LOCATOR = optarg;
        break;
      case 'D':
        LOG_DEBUG = true;
        break;
      case 'V':
        LOG_VERBOSE = true;
        break;
      case 'v':
        printf("%s\n", PACKAGE_STRING);
        exit(EXIT_SUCCESS);
      case 'h':
        printf("%s:\n\n%s\n", PACKAGE_STRING, OPTIONS);
        exit(EXIT_SUCCESS);
      default:
        exit(EXIT_FAILURE);
    }
  }
}

static void SetupDebugMessenger(void) {
  LCH_DebugMessengerInitInfo initInfo = {
      .severity = LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT |
                  LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT |
                  LCH_DEBUG_MESSAGE_TYPE_INFO_BIT,
      .messageCallback = &LCH_DebugMessengerCallbackDefault,
  };
  if (LOG_VERBOSE) {
    initInfo.severity |= LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT;
  }
  if (LOG_DEBUG) {
    initInfo.severity |= LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT;
  }
  LCH_DebugMessengerInit(&initInfo);
}

static LCH_Instance *SetupInstance(void) {
  LCH_Instance *instance = NULL;
  {  // Create instance
    LCH_InstanceCreateInfo createInfo = {
        .instanceID = UNIQUE_ID,
        .workDir = WORK_DIR,
    };

    instance = LCH_InstanceCreate(&createInfo);
    if (instance == NULL) {
      LCH_LOG_ERROR("LCH_InstanceCreate");
      return NULL;
    }
  }

  {  // Add CSV table
    LCH_TableCreateInfo createInfo = {
        .readLocator = READ_LOCATOR,
        .readCallback = LCH_TableReadCallbackCSV,
        .writeLocator = WRITE_LOCATOR,
        .writeCallback = LCH_TableWriteCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      LCH_LOG_ERROR("LCH_TableCreate");
      return NULL;
    }

    // TODO: Add table to instance
    LCH_TableDestroy(table);
  }

  return instance;
}
