/*****シングルクライアント*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//エラーメッセージ
void err_func(char *msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
  struct sockaddr_in serv;
  struct sockaddr_in clt;
  socklen_t sin_size;
  unsigned int sockfd,new_sockfd;
  //char addr[]="192.168.1.6";
  char addr[]="192.168.2.103";
  unsigned short port=10000;
  int len,i,tmp=0;
  char buf[BUFSIZ],pagename[100],htmldata[BUFSIZ],html[BUFSIZ]="\0",str[100];
  char ch;
  char *p;
  FILE *fp=NULL;

  //実行ファイル名とPOP3サーバアドレスの2つの要素が入力されているかチェック
  if(argc != 1){
    printf("usage: progname serv_ip serv_port\n");
    exit(EXIT_FAILURE);
  }

  //socketシステムコール
  if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    err_func("socket");

  //IPアドレスとポート番号をsockaddr_in構造体に格納
  serv.sin_family = PF_INET;
  serv.sin_port = htons(port);
  inet_aton(addr, &(serv.sin_addr));

  //bindシステムコール
  if((bind(sockfd, (struct sockaddr *)&serv, sizeof(serv)))<0)
    err_func("bind");

  //listenシステムコール
  if((listen(sockfd, SOMAXCONN))<0)
    err_func("listen");
  printf("サーバが接続待機状態となりました\n【仮処理】URLのファイル名を\"exit\"と指定することでサーバを終了させます\n");
  while(1){
    //acceptシステムコール
    sin_size = sizeof(struct sockaddr_in);
    if((new_sockfd = accept(sockfd, (struct sockaddr *)&clt, &sin_size))<0)
      err_func("accept");
    len = recv(new_sockfd, buf, BUFSIZ, 0);  //リクエスト受信
    buf[len]='\0';

    //リクエストの解析
    p=buf;
    //リクエストされたファイル名を取得
    while(*p!='/')
      p++;
    p++;
    i=0;
    if(*p==' ')  strcpy(pagename,"index.html");  // '/'で終わるリクエストはindex.htmlを指定
    else{
      while(1){
        pagename[i]=*p;
        if(*(p+1)==' ')  break;
        p++;
        i++;
      }
      pagename[i+1]='\0';
    }
    if(strcmp(pagename,"exit")==0)  break;  //URLのファイル名を"exit"と指定したときにサーバを終了させる
    //ファイル読み込み
    i=0;
    fp = fopen(pagename, "r");
    if(fp==NULL){  //ファイルがオープンできなかった場合（=404NotFoundを返す）
      strcat(html,"HTTP/1.1 404 Not Found\r\n");  //404 Not Foundを返す
      strcpy(htmldata,
        "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
        //404用ページのhtmlデータ
      len=strlen(htmldata);  //htmlデータの長さを調べる
      sprintf(str,"Content-Length: %d\r\n",len);  //htmlデータの長さを出力
      strcat(html,str);
      strcat(html,"\r\n");
      strcat(html,htmldata);

      //sendシステムコール
      len=strlen(html);
      send(new_sockfd, html, len, 0);
      printf("%d回目のリクエストに対するレスポンスを完了(404 Not Found)\n",++tmp);
    }else{  //ファイルがオープンできた場合(=200OKを返す)
      while ((ch = fgetc(fp))!=EOF){
        htmldata[i]=ch;
        i++;
      }
      htmldata[i]='\0';
      fclose(fp);

      strcat(html,"HTTP/1.1 200 OK\r\n");  //200 OKを返す
      len=strlen(htmldata);  //htmlデータの長さを調べる
      sprintf(str,"Content-Length: %d\r\n",len);  //htmlデータの長さを出力
      strcat(html,str);
      strcat(html,"\r\n");
      strcat(html,htmldata);

      //sendシステムコール
      len=strlen(html);
      send(new_sockfd, html, len, 0);
      printf("%d回目のリクエストに対するレスポンスを完了(200 OK)\n",++tmp);
    }
    strcpy(str,"\0");
    strcpy(html,"\0");
    close(new_sockfd);  //socketクローズ
  }
  printf("サーバを終了します\n");
  close(sockfd);  //socketクローズ
  return 0;
}
