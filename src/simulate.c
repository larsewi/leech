#include <assert.h>
#include <errno.h>
#include <leech.h>
#include <leech_csv.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "2022"
#define WORK_DIR ".leech/"
#define BACKLOG 10
#define MAX_EVENTS 10

typedef struct CommandParams {
  LCH_Instance *instance;
  LCH_List *arguments;
  bool success;
} CommandParams;

static char *UNIQUE_ID = NULL;
static bool SHOULD_RUN = true;
static bool LOG_DEBUG = false;
static bool LOG_VERBOSE = false;

static void CheckOptions(int argc, char *argv[]);
static void SetupDebugMessenger(void);
static LCH_Instance *SetupInstance(void);
static int CreateServerSocket(void);
static LCH_Dict *SetupCommands(void);
static bool ParseCommand(LCH_Instance *instance, LCH_Dict *commands,
                         const char *command);
static void ExitCommand(CommandParams *params);
static void BootstrapCommand(CommandParams *params);

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

  LCH_Dict *commands = SetupCommands();
  if (commands == NULL) {
    close(server_sock);
    LCH_InstanceDestroy(instance);
  }

  char buffer[LCH_BUFFER_SIZE];
  ssize_t size;
  while (SHOULD_RUN) {
    int ret = poll(pfds, LCH_LENGTH(pfds), -1);
    if (ret == -1) {
      LCH_LOG_ERROR("poll: %s", strerror(errno));
      LCH_DictDestroy(commands);
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
          LCH_DictDestroy(commands);
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
          LCH_DictDestroy(commands);
          close(server_sock);
          LCH_InstanceDestroy(instance);
          return EXIT_FAILURE;
        }
        if (size == 0) {
          SHOULD_RUN = false;
          break;
        }
        buffer[size] = '\0';
        if (!ParseCommand(instance, commands, buffer)) {
          LCH_DictDestroy(commands);
          close(server_sock);
          LCH_InstanceDestroy(instance);
          return EXIT_FAILURE;
        }
      }
    }
  }

  LCH_DictDestroy(commands);
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
      perror("strdup");
      return NULL;
    }

    char *workDir = strdup(WORK_DIR);
    if (workDir == NULL) {
      perror("strdup");
      free(instanceID);
      return NULL;
    }

    LCH_InstanceCreateInfo createInfo = {
        .instanceID = instanceID,
        .workDir = workDir,
    };

    instance = LCH_InstanceCreate(&createInfo);
    if (instance == NULL) {
      fprintf(stderr, "LCH_InstanceCreate\n");
      free(workDir);
      free(instanceID);
      return NULL;
    }
  }

  { // Add CSV table
    char *readLocator = strdup("client/example.csv");
    if (readLocator == NULL) {
      perror("strdup");
      return NULL;
    }

    char *writeLocator = strdup("server/example.csv");
    if (writeLocator == NULL) {
      perror("strdup");
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

static int CreateServerSocket(void) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *info;
  int rc = getaddrinfo(NULL, PORT, &hints, &info);
  if (rc == -1) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    return -1;
  }

  struct addrinfo *ptr;
  int yes = 1;
  for (ptr = info; ptr != NULL; ptr = ptr->ai_next) {
    int fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (fd == -1) {
      perror("socket");
      continue;
    }

    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc == -1) {
      perror("setsockopt");
      close(fd);
      freeaddrinfo(info);
      return -1;
    }

    rc = bind(fd, ptr->ai_addr, ptr->ai_addrlen);
    if (rc == -1) {
      perror("bind");
      close(fd);
      continue;
    }

    freeaddrinfo(info);

    rc = listen(fd, BACKLOG);
    if (rc == -1) {
      perror("listen");
      close(fd);
      return -1;
    }

    return fd;
  }

  freeaddrinfo(info);
  fprintf(stderr, "Failed to bind\n");

  return -1;
}

static LCH_Dict *SetupCommands(void) {
  LCH_Dict *commands = LCH_DictCreate();
  if (commands == NULL) {
    LCH_LOG_ERROR("LCH_DictCreate: %s", strerror(errno));
    return NULL;
  }

  LCH_DictSet(commands, "exit", NULL, NULL, (void (*)(void *)) ExitCommand);
  LCH_DictSet(commands, "bootstrap", NULL, NULL, (void (*)(void *)) BootstrapCommand);

  return commands;
}

static bool ParseCommand(LCH_Instance *instance, LCH_Dict *cmds,
                         const char *str) {
  LCH_List *args = LCH_SplitString(str, " \t\n");

  if (LCH_ListLength(args) == 0) {
    LCH_ListDestroy(args);
    return true;
  }

  char *cmd_str = (char *)LCH_ListGet(args, 0, NULL);
  if (!LCH_DictHasKey(cmds, cmd_str)) {
    LCH_LOG_INFO("Bad command '%s'", cmd_str);
    LCH_ListDestroy(args);
    return true;
  }

  void (*func)(CommandParams *) = NULL;
  LCH_DictGet(cmds, cmd_str, (void (**)(void *)) &func);
  assert(func != NULL);

  CommandParams params = {
    .instance = instance,
    .arguments = args,
    .success = false,
  };

  func(&params);
  LCH_ListDestroy(args);

  return params.success;
}

static void ExitCommand(CommandParams *params) {
  SHOULD_RUN = false;
  params->success = true;
}

static void BootstrapCommand(CommandParams *params) {
  if (LCH_ListLength(params->arguments) < 2) {
    LCH_LOG_ERROR("Missing argument <ip-address>");
  }
  else {
    char *ip = LCH_ListGet(params->arguments, 1, NULL);
    LCH_LOG_DEBUG("Bootstrap ip '%s'",ip);
  }

  params->success = true;
}
