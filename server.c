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
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
      close(sockfd);
      err_reason = "bind";
      continue;
    }
    // if code reaches here we're unable to create a socket and connect.
    break;
  }
  if (sockfd < 0) {
    fprintf(stderr, "connection error: %s\n", err_reason);
    exit(1);
  }

  // free address structs
  freeaddrinfo(server_info);

  if (listen(sockfd, BACKLOG) < 0) {
    perror("error listening");
    exit(1);
  }
  printf("Listening on port %s...\n", port_number);

  client_addrlen = sizeof client_addr;
  new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);

  while (1) {
    memset(buffer, 0, BUF_SIZE);

    // read incoming data from client
    if (read(new_sockfd, buffer, BUF_SIZE) < 0) {
      perror("error reading data");
      continue;
    }
    printf("client: %s", buffer);

    // clear buffer and write into it
    memset(buffer, 0, BUF_SIZE);
    printf("server: ");
    fgets(buffer, BUF_SIZE, stdin);
    if (write(new_sockfd, buffer, BUF_SIZE) < 0) {
      perror("error writing data");
      continue;
    }
  }
  close(new_sockfd);
  close(sockfd);
  return 0;
}
