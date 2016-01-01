#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define STDIN_FD 0

int select_func(int sockfd);

void err_func(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int sockfd, len;
	char buf[BUFSIZ];
	struct sockaddr_in serv;
	unsigned short port;
   
	if(argc != 3)
	{
		printf("usage: progname serv_ip serv_port\n");
		exit(EXIT_FAILURE);
	}
	
	if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_func("socket");
   
	serv.sin_family = PF_INET;
	port = (unsigned short)atoi(argv[2]);
	serv.sin_port = htons(port);
	inet_aton(argv[1], &(serv.sin_addr));
   
	if(connect(sockfd, (struct sockaddr *)&serv, sizeof(struct sockaddr_in)) < 0)
		err_func("connect");
   
	do
	{
		if(select_func(sockfd) == sockfd)
		{
			len = recv(sockfd, buf, BUFSIZ, 0);
			buf[len] = '\0';
			printf("<- %s\n", buf);
		}
		else
		{
			len = read(STDIN_FD, buf, BUFSIZ);
			len = send(sockfd, buf, len, 0);
		}
	}while(strncmp(buf, "EXIT\r\n", 6) != 0 && strncmp(buf, "EXIT\n", 5) != 0);
	
	close(sockfd);
	return 0;
}

int select_func(int sockfd)
{
	fd_set rfds;
   
	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);
	FD_SET(STDIN_FD, &rfds);
	
	if(select(sockfd+1, &rfds, NULL, NULL, NULL) < 0)
		err_func("select");
	
	if(FD_ISSET(STDIN_FD, &rfds))
		return STDIN_FD;
	
	return sockfd;
}
