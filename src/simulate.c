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
#define BUFSIZE 4096
#define LENGTH(x) (sizeof(x) / sizeof(*x))

static char *UNIQUE_ID = NULL;
static bool SHOULD_RUN = true;
static bool LOG_DEBUG = false;
static bool LOG_VERBOSE = false;

static void CheckOptions(int argc, char *argv[]);
LCH_Instance *SetupInstance();
static int CreateServerSocket();
static bool ParseCommand(LCH_Instance *instance, const char *command);

int main(int argc, char *argv[]) {
  CheckOptions(argc, argv);

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

  char buffer[BUFSIZ];
  ssize_t size;
  while (SHOULD_RUN) {
    int ret = poll(pfds, LENGTH(pfds), -1);
    if (ret == -1) {
      perror("poll");
      close(server_sock);
      LCH_InstanceDestroy(instance);
      return EXIT_FAILURE;
    }

    for (int i = 0; i < LENGTH(pfds); i++) {
      struct pollfd *pfd = pfds + i;
      if (pfd->revents != POLLIN) {
        continue;
      }

      if (pfd->fd == server_sock) {
        size = read(server_sock, (void *)buffer, BUFSIZ);
        if (size < 0) {
          close(server_sock);
          LCH_InstanceDestroy(instance);
          perror("read");
          return EXIT_FAILURE;
        }
        printf("Handle server sock: %s\n", buffer);
      }

      else if (pfd->fd == STDIN_FILENO) {
        size = read(STDIN_FILENO, (void *)buffer, BUFSIZ);
        if (size < 0) {
          close(server_sock);
          LCH_InstanceDestroy(instance);
          perror("read");
          return EXIT_FAILURE;
        }
        if (size == 0) {
          printf("Exited by user\n");
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

LCH_Instance *SetupInstance() {
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
      return NULL;
    }

    LCH_InstanceCreateInfo createInfo = {
        .instanceID = instanceID,
        .workDir = workDir,
    };

    instance = LCH_InstanceCreate(&createInfo);
    if (instance == NULL) {
      fprintf(stderr, "LCH_InstanceCreate\n");
      return NULL;
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
      return NULL;
    }

    LCH_TableCreateInfo createInfo = {
        .readLocator = readLocator,
        .readCallback = LCH_TableReadCallbackCSV,
        .writeLocator = writeLocator,
        .writeCallback = LCH_TableWriteCallbackCSV,
    };

    if (!LCH_TableAdd(instance, &createInfo)) {
      fprintf(stderr, "LCH_TableAdd\n");
      LCH_InstanceDestroy(instance);
      return NULL;
    }
  }

  return instance;
}

static int CreateServerSocket() {
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

static bool ParseCommand(LCH_Instance *instance, const char *command) {
  return true;
}
