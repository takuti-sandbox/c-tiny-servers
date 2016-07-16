#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void err_func(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc , char **argv) {
  int sockfd, len;
  char sbuf[BUFSIZ];
  struct sockaddr_in server, client;
  socklen_t addr_size = sizeof(struct sockaddr);

  if (argc != 3) err_func("Wrong usage `$ progname serv_ip serv_port`");

  // open socket
  if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) err_func("Cannot open socket");

  // [src] client setting as localhost:1024
  client.sin_family = PF_INET;
  client.sin_port = htons(1024);
  inet_aton("127.0.0.1", &(client.sin_addr));

  // bind the specified client IP/port to the opening socket
  if (bind(sockfd, (struct sockaddr *)&client, sizeof(client)) < 0) err_func("Cannot bind server");

  // [dst] sever setting based on given command-line arguments
  server.sin_family = PF_INET;
  server.sin_port = htons((unsigned short)atoi(argv[2]));
  inet_aton(argv[1], &(server.sin_addr));

  while (1) {
    printf("> ");
    scanf("%s", sbuf);

    if(strcmp(sbuf, "EXIT") == 0) break;

    // send a message to a server
    len = strlen(sbuf);
    sendto(sockfd, sbuf, len, 0, (struct sockaddr *)&server, addr_size);

    // receive a message from a server
    len = recvfrom(sockfd, sbuf, BUFSIZ - 1, 0, (struct sockaddr *)&server, &addr_size);
    sbuf[len] = '\0';

    printf("%s\n", sbuf);
  }

  close(sockfd);

  return 0;
}
