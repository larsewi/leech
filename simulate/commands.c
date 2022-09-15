#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "commands.h"

#define PORT "2022"

typedef struct Command {
  char *key;
  bool (*func)(LCH_Instance *, const LCH_List *);
} Command;

static bool Bootstrap(LCH_Instance *const instance, const LCH_List *const args);
static bool Commit(LCH_Instance *const instance, const LCH_List *const args);
static bool Fetch(LCH_Instance *const instance, const LCH_List *const args);

static const Command commands[] = {
    {"bootstrap", Bootstrap},
    {"connect", Commit},
    {"fetch", Fetch},
};

bool ParseCommand(LCH_Instance *const instance, const char *const str) {
  LCH_List *args = LCH_SplitString(str, " \t\n");
  if (args == NULL) {
    LCH_ListDestroy(args);
    return false;
  }

  if (LCH_ListLength(args) < 1) {
    LCH_ListDestroy(args);
    return true;
  }

  char *command = LCH_ListGet(args, 0);
  if (command == NULL) {
    LCH_ListDestroy(args);
    return false;
  }

  int len = LCH_LENGTH(commands);
  for (int i = 0; i < len; i++) {
    if (strcmp(command, commands[i].key) == 0) {
      const bool ret = commands[i].func(instance, args);
      LCH_ListDestroy(args);
      return ret;
    }
  }

  LCH_LOG_ERROR("Bad command '%s'", command);
  return true;
}

static bool Bootstrap(LCH_Instance *const instance,
                      const LCH_List *const args) {
  if (LCH_ListLength(args) < 2) {
    LCH_LOG_ERROR("Missing argument 'ip address'");
    return true;
  }

  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  const char *const ip = (const char *)LCH_ListGet(args, 1);

  struct addrinfo *info = NULL;
  int ret = getaddrinfo(ip, PORT, &hints, &info);
  if (ret == -1) {
    LCH_LOG_ERROR("getaddrinfo: %s", gai_strerror(ret));
    return false;
  }

  int sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  if (sock == -1) {
    LCH_LOG_ERROR("socket: %s", strerror(errno));
    freeaddrinfo(info);
    return false;
  }

  LCH_LOG_DEBUG("Connecting to %s:%s", ip, PORT);
  ret = connect(sock, info->ai_addr, info->ai_addrlen);
  if (ret == -1) {
    LCH_LOG_ERROR("socket: %s", strerror(errno));
    freeaddrinfo(info);
    return false;
  }

  freeaddrinfo(info);

  return true;
}

static bool Commit(LCH_Instance *const instance, const LCH_List *const args) {
  LCH_LOG_DEBUG("Commit command called!");
  return true;
}

static bool Fetch(LCH_Instance *const instance, const LCH_List *const args) {
  LCH_LOG_DEBUG("Fetch command called!");
  return true;
}
