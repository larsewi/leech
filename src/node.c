#include <assert.h>
#include <ctype.h>
#include <leech.h>
#include <leech_csv.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "server.h"
#include "client.h"

#define LENGTH(x) (sizeof(x) / sizeof(*x))

static char *BOOTSTRAP_ADDRESS = NULL;
static char *BOOTSTRAP_PORT = NULL;
static bool LOG_DEBUG = false;
static bool LOG_VERBOSE = false;

static void CheckOpts(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  CheckOpts(argc, argv);

  LCH_Instance *instance = NULL;
  { // Create instance
    LCH_InstanceCreateInfo createInfo = {
        .instanceID = BOOTSTRAP_ADDRESS,
        .workDir = ".leech/",
    };
    if ((instance = LCH_InstanceCreate(&createInfo)) == NULL) {
      fprintf(stderr, "LCH_InstanceCreate\n");
      return EXIT_FAILURE;
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
      LCH_InstanceDestroy(instance);
      return EXIT_FAILURE;
    }
  }

  { // Add CSV table
    LCH_TableCreateInfo createInfo = {
        .readLocator = "client/example.csv",
        .readCallback = LCH_TableReadCallbackCSV,
        .writeLocator = "server/example.csv",
        .writeCallback = LCH_TableWriteCallbackCSV,
    };
    if (!LCH_TableAdd(instance, &createInfo)) {
      fprintf(stderr, "LCH_TableAdd\n");
      LCH_InstanceDestroy(instance);
      return EXIT_FAILURE;
    }
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    LCH_InstanceDestroy(instance);
    return EXIT_FAILURE;
  }
  else if (pid == 0) {
    if (!Server(instance, BOOTSTRAP_PORT)) {
      LCH_InstanceDestroy(instance);
      return EXIT_FAILURE;
    }
  }
  else {
    if (!Client(instance, BOOTSTRAP_ADDRESS, BOOTSTRAP_PORT)) {
      LCH_InstanceDestroy(instance);
      return EXIT_FAILURE;
    }

    kill(pid, SIGTERM);
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
  }

  LCH_InstanceDestroy(instance);
  return EXIT_SUCCESS;
}

static void CheckOpts(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "b:vdh")) != -1) {
    switch (opt) {
    case 'b':
      char *sep = strrchr(optarg, ':');
      if (sep == NULL) {
        fprintf(stderr, "Bad address '%s'\n", optarg);
        exit(EXIT_FAILURE);
      }
      *sep++ = '\0';
      BOOTSTRAP_ADDRESS = optarg;
      BOOTSTRAP_PORT = sep;
      break;
    case 'd':
      LOG_DEBUG = true;
      break;
    case 'v':
      LOG_VERBOSE = true;
      break;
    case 'h':
      printf("%s -b IP:PORT [-d] [-v] [-h]\n", argv[0]);
      exit(EXIT_SUCCESS);
    default:
      exit(EXIT_FAILURE);
    }
  }
}
