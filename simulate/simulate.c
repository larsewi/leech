#include <assert.h>
#include <errno.h>
#include <leech.h>
#include <leech_csv.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"
#include "server.h"

#define WORK_DIR ".leech/"
#define MAX_EVENTS 10

static char *UNIQUE_ID = NULL;
static bool SHOULD_RUN = true;
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

  int server_sock = CreateServerSocket();
  if (server_sock == -1) {
    LCH_InstanceDestroy(instance);
    return EXIT_FAILURE;
  }

  struct pollfd pfds[] = {
      {
          .fd = server_sock,
          .events = POLLIN,
      },
      {
          .fd = STDIN_FILENO,
          .events = POLLIN,
      },
  };

  char buffer[LCH_BUFFER_SIZE];
  ssize_t size;
  while (SHOULD_RUN) {
    int ret = poll(pfds, LCH_LENGTH(pfds), -1);
    if (ret == -1) {
      LCH_LOG_ERROR("poll: %s", strerror(errno));
      close(server_sock);
      LCH_InstanceDestroy(instance);
      return EXIT_FAILURE;
    }

    for (int i = 0; i < LCH_LENGTH(pfds); i++) {
      struct pollfd *pfd = pfds + i;
      if (pfd->revents != POLLIN) {
        continue;
      }

      if (pfd->fd == server_sock) {
        LCH_LOG_DEBUG("Handling server socket event");
        size = read(server_sock, (void *)buffer, sizeof(buffer));
        if (size < 0) {
          LCH_LOG_ERROR("read: %s", strerror(errno));
          close(server_sock);
          LCH_InstanceDestroy(instance);
          return EXIT_FAILURE;
        }
      }

      else if (pfd->fd == STDIN_FILENO) {
        LCH_LOG_DEBUG("Handling 'stdin' file descriptor event");
        size = read(STDIN_FILENO, (void *)buffer, sizeof(buffer));
        if (size < 0) {
          LCH_LOG_ERROR("read: %s", strerror(errno));
          close(server_sock);
          LCH_InstanceDestroy(instance);
          return EXIT_FAILURE;
        }
        if (size == 0) {
          LCH_LOG_DEBUG("Exited by user");
          SHOULD_RUN = false;
          break;
        }
        buffer[size] = '\0';
        if (!ParseCommand(instance, buffer)) {
          close(server_sock);
          LCH_InstanceDestroy(instance);
          return EXIT_FAILURE;
        }
      }
    }
  }

  close(server_sock);
  LCH_InstanceDestroy(instance);
  return EXIT_SUCCESS;
}

static void CheckOptions(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "vdh")) != -1) {
    switch (opt) {
    case 'd':
      LOG_DEBUG = true;
      break;
    case 'v':
      LOG_VERBOSE = true;
      break;
    case 'h':
      printf("usage: %s UNIQUE_ID [-d] [-v] [-h]\n", argv[0]);
      exit(EXIT_SUCCESS);
    default:
      exit(EXIT_FAILURE);
    }
  }
  if (optind < argc) {
    UNIQUE_ID = argv[optind++];
  } else {
    fprintf(stderr, "Missing required argument 'UNIQUE_ID'\n");
    exit(EXIT_FAILURE);
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
  { // Create instance
    char *instanceID = strdup(UNIQUE_ID);
    if (instanceID == NULL) {
      LCH_LOG_ERROR("strdup: %s", strerror(errno));
      return NULL;
    }

    char *workDir = strdup(WORK_DIR);
    if (workDir == NULL) {
      LCH_LOG_ERROR("strdup: %s", strerror(errno));
      free(instanceID);
      return NULL;
    }

    LCH_InstanceCreateInfo createInfo = {
        .instanceID = instanceID,
        .workDir = workDir,
    };

    instance = LCH_InstanceCreate(&createInfo);
    if (instance == NULL) {
      LCH_LOG_ERROR("LCH_InstanceCreate: %s", strerror(errno));
      free(workDir);
      free(instanceID);
      return NULL;
    }
  }

  { // Add CSV table
    char *readLocator = strdup("client/example.csv");
    if (readLocator == NULL) {
      LCH_LOG_ERROR("strdup: %s", strerror(errno));
      return NULL;
    }

    char *writeLocator = strdup("server/example.csv");
    if (writeLocator == NULL) {
      LCH_LOG_ERROR("strdup: %s", strerror(errno));
      free(readLocator);
      return NULL;
    }

    LCH_TableCreateInfo createInfo = {
        .readLocator = readLocator,
        .readCallback = LCH_TableReadCallbackCSV,
        .writeLocator = writeLocator,
        .writeCallback = LCH_TableWriteCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      free(writeLocator);
      free(readLocator);
      LCH_InstanceDestroy(instance);
      return NULL;
    }

    // TODO: Add table to instance
    LCH_TableDestroy(table);
  }

  return instance;
}
