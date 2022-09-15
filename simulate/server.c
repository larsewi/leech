#include <errno.h>
#include <leech.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "2022"
#define BACKLOG 10

#include "server.h"

int CreateServerSocket(void) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *info;
  int rc = getaddrinfo(NULL, PORT, &hints, &info);
  if (rc == -1) {
    LCH_LOG_ERROR("getaddrinfo: %s", gai_strerror(rc));
    return -1;
  }

  struct addrinfo *ptr;
  int yes = 1;
  for (ptr = info; ptr != NULL; ptr = ptr->ai_next) {
    int fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (fd == -1) {
      LCH_LOG_ERROR("socket: %s", strerror(errno));
      continue;
    }

    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc == -1) {
      LCH_LOG_ERROR("setsockopt: %s", strerror(errno));
      close(fd);
      freeaddrinfo(info);
      return -1;
    }

    rc = bind(fd, ptr->ai_addr, ptr->ai_addrlen);
    if (rc == -1) {
      LCH_LOG_ERROR("bind: %s", strerror(errno));
      close(fd);
      continue;
    }

    freeaddrinfo(info);

    rc = listen(fd, BACKLOG);
    if (rc == -1) {
      LCH_LOG_ERROR("listen: %s", strerror(errno));
      close(fd);
      return -1;
    }

    return fd;
  }

  freeaddrinfo(info);
  LCH_LOG_ERROR("Failed to bind");

  return -1;
}
