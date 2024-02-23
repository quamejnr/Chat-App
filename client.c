#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 10
#define BUF_SIZE 255

int main(int argc, char *argv[]) {
  char *port_number = argv[1];
  int error, sockfd, new_sockfd;
  char *err_reason;
  struct addrinfo hints, *res, *server_info;
  struct sockaddr_storage client_addr;
  socklen_t client_addrlen;
  char buffer[BUF_SIZE];

  if (argc < 2) {
    fprintf(stderr, "usage %s: provide port number\n", argv[0]);
    return 1;
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // Fill with host ip address

  if ((error = getaddrinfo(NULL, port_number, &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(error));
    exit(1);
  }

  for (res = server_info; res; res = res->ai_next) {
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) <
        0) {
      err_reason = "socket";
      continue;
    }
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
      close(sockfd);
      err_reason = "conect";
      continue;
    }
    break;
  }
  if (sockfd < 0) {
    fprintf(stderr, "connection error: %s\n", err_reason);
    exit(1);
  }

  printf("connected to %s...\n", port_number);

  // free address structs
  freeaddrinfo(server_info);

  while (1) {

    // write data to server
    memset(buffer, 0, BUF_SIZE);
    printf("client: ");
    fgets(buffer, BUF_SIZE, stdin);
    if (write(sockfd, buffer, BUF_SIZE) < 0) {
      perror("error writing data");
      continue;
    }

    // read incoming data from server
    memset(buffer, 0, BUF_SIZE);
    if (read(sockfd, buffer, BUF_SIZE) < 0) {
      perror("error reading data");
      continue;
    }
    printf("server: %s", buffer);
  }
  close(sockfd);
  return 0;
}
