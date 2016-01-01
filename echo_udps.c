#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//エラー処理関数
void err_func(char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc , char **argv){
  int sockfd;
  char sbuf[BUFSIZ];
  int len;
  struct sockaddr_in serv;
  struct sockaddr_in sin;
  socklen_t sin_size;
  struct sockaddr_in clt;
  char addr[]="127.0.0.1";
  unsigned short port=1024;
  unsigned short myport;

  //コンパイルするときはパス・IP・ポート番号の３つを入力させたいのでargcは３になる
  if(argc != 3)
  {
    printf("usage: progname serv_ip serv_port\n");
    exit(EXIT_FAILURE);
  }

  //socketシステムコール
  if((sockfd=socket(PF_INET,SOCK_DGRAM,0))<0)
    err_func("socket");

  //bindシステムコール
  sin.sin_family = PF_INET;                                   /* IPv4を指定*/
  myport=(unsigned short)atoi(argv[2]);
  sin.sin_port = htons(myport);                                 /* ポート番号の設定 */
  inet_aton(argv[1], &(sin.sin_addr));                          /* IPアドレスの設定 */

  if(bind(sockfd, (struct sockaddr *)&sin, sizeof(sin))<0)
    err_func("bind");

  while(1){
  //recvfromシステムコール
  sin_size = sizeof(struct sockaddr_in);
  len = recvfrom(sockfd, sbuf, 255, 0, (struct sockaddr *)&clt, &sin_size);

  sbuf[len]='\0';
  printf("message from %s(%d)：%s\n",addr,port,sbuf);

  //sendtoシステムコール
  serv.sin_family = PF_INET;
  serv.sin_port = htons(port);
  inet_aton(addr, &(serv.sin_addr));

  len = strlen(sbuf);
  sendto(sockfd, sbuf, len, 0, (struct sockaddr *)&serv, sizeof(struct sockaddr));
  }

  close(sockfd);//ソケットの開放

  return 0;
}
