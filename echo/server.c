#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "echo.h"

int main(int argc , char **argv) {
  int sockfd, len;
  unsigned short server_port;
  char sbuf[BUFSIZ], s_return[BUFSIZ + 100];
  char *server_addr;
  struct sockaddr_in server, client;
  socklen_t addr_size = sizeof(struct sockaddr);

  if (argc != 3) err_func("Wrong usage `$ progname serv_ip serv_port`");

  // open socket
  if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) err_func("Cannot open socket");

  // [src] server setting based on given command-line arguments
  server.sin_family = PF_INET;
  server_port = (unsigned short)atoi(argv[2]);
  server.sin_port = htons(server_port);
  server_addr = &argv[1][0];
  inet_aton(server_addr, &(server.sin_addr));

  // bind the specified server IP/port to the opening socket
  if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) err_func("Cannot bind server");

  printf("Server %s:%d is now running.\n", server_addr, server_port);

  // [dst] client setting as localhost:1024
  client.sin_family = PF_INET;
  client.sin_port = htons(1024);
  inet_aton("127.0.0.1", &(client.sin_addr));

  while (1) {
    // receive a message from a client
    len = recvfrom(sockfd, sbuf, BUFSIZ - 1, 0, (struct sockaddr *)&client, &addr_size);
    sbuf[len] = '\0';

    printf("Message from localhost: %s\n", sbuf);

    // send (return) a message to localhost
    sprintf(s_return, "[%s:%d] %s", server_addr, server_port, sbuf);
    sendto(sockfd, s_return, strlen(s_return), 0, (struct sockaddr *)&client, addr_size);
  }

  close(sockfd);

  return 0;
}
