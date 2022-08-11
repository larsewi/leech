#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#include "server.h"

#define BACKLOG 10

static bool SHOULD_RUN = true;

static void SIGCHLD_Handler(int signum)
{
  int saved_errno = errno;
  while(waitpid(-1, NULL, WNOHANG) > 0);
  errno = saved_errno;
}

static void SIGTERM_Handler(int signum)
{
  SHOULD_RUN = false;
}

bool Server(LCH_Instance *instance, const char *port) {
  // Setup SIGCHLD handler
  int rc = signal(SIGCHLD, SIGCHLD_Handler);
  if (rc == -1) {
    perror("server: signal");
    return false;
  }

  // Setup SIGTERM handler
  rc = signal(SIGCHLD, SIGTERM_Handler);
  if (rc == -1) {
    perror("server: signal");
    return false;
  }

  // Setup socket
  int sock = -1;
  {
    struct addrinfo hints = {0}, *info = NULL, *p = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(NULL, port, &hints, &info);
    if (rc != 0) {
      fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rc));
      freeaddrinfo(info);
      return false;
    }

    int yes = 1;
    for (p = info; p != NULL; p = p->ai_next) {
      sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (sock == -1) {
        perror("server: socket");
        continue;
      }

      rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      if (rc == -1) {
        perror("server: setsockopt");
        freeaddrinfo(info);
        close(sock);
        return false;
      }

      rc = bind(sock, p->ai_addr, p->ai_addrlen);
      if (rc == -1) {
        perror("server: bind");
        close(sock);
        continue;
      }
      break;
    }
    freeaddrinfo(info);

    if (p == NULL) {
      fprintf(stderr, "server: Failed to bind\n");
      return false;
    }
  }

  rc = listen(sock, BACKLOG);
  if (rc == -1) {
    perror("server: listen");
    close(sock);
    return false;
  }

  {
    struct sockaddr_storage addr;
    socklen_t size = sizeof(addr);

    while (SHOULD_RUN) {
      int new_sock = accept(sock, (struct sockaddr *) &addr, &size);
      if (new_sock == -1) {
        perror("server: accept");
        continue;
      }

      pid_t pid = fork();
      if (pid == -1) {
        perror("server: fork");
        close(new_sock);
        close(sock);
        return false;
      }
      else if (pid == 0) {
        close(sock);

        // TODO: read / write

        close(new_sock);
      }
      close(new_sock);
    }
  }

  return true;
}