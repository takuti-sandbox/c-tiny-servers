/*****マルチクライアント*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void child_process(int accept_sock);
void signal_handler(int sig);

void err_func(char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  int listen_sock, accept_sock, pid;
  socklen_t sin_siz;
  struct sockaddr_in serv, clt;
  //char addr[]="192.168.1.7";
  char addr[]="192.168.2.100";
  unsigned short port=10000;

  signal(SIGCHLD, signal_handler);

  if(argc != 1)
  {
    printf("usage: progname ip port\n");
    exit(EXIT_FAILURE);
  }

  //socketシステムコール
  if((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    err_func("socket");

  //IPアドレスとポート番号をsockaddr_in構造体に格納
  serv.sin_family = PF_INET;
  serv.sin_port = htons(port);
  inet_aton(addr, &(serv.sin_addr));

  //bindシステムコール
  if((bind(listen_sock, (struct sockaddr *)&serv, sizeof(serv)))<0)
    err_func("bind");

  while(1)
  {
    if(listen(listen_sock, SOMAXCONN) < 0)
      err_func("listen");
    if((accept_sock = accept(listen_sock, (struct sockaddr *)&clt, &sin_siz)) < 0)
      err_func("accept");

    if((pid = fork()) == 0)  //子プロセスであれば処理を行う
    {
      close(listen_sock);
      child_process(accept_sock);
    }
    close(accept_sock);
  }
  close(listen_sock);
  return 0;
}

void child_process(int accept_sock)  //子プロセスの処理
{
  int len=0,i;
  char buf[BUFSIZ],filename[100],data[BUFSIZ],html[BUFSIZ]="\0",type[]="text/html";
  char ch;
  char *p;
  FILE *fp=NULL;

  len =recv(accept_sock, buf, BUFSIZ, 0);  //リクエスト受信
  buf[len]='\0';

  //リクエストの解析
  p=buf;
  //リクエストされたファイル名を取得
  while(*p!='/')
    p++;
  p++;
  i=0;
  if(*p==' ')  strcpy(filename,"index.html");  // '/'で終わるリクエストはindex.htmlを指定
  else{
    while(1){
      filename[i]=*p;
      if(*(p+1)==' ')  break;
      p++;
      i++;
    }
    filename[i+1]='\0';
  }
  //ファイル読み込み
  i=0;
  if(strstr(filename,".html")!=NULL){  //htmlファイルを要求された場合
    fp = fopen(filename, "r");
    if(fp==NULL){  //ファイルがオープンできなかった場合（=404NotFoundを返す）
      strcpy(data,
        "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
        //404用ページのhtmlデータ
      len=strlen(data);  //htmlデータの長さを調べる
      //レスポンスの作成
      sprintf(html,
        "HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n%s",len,type,data);
      //レスポンス（HTMLデータ）の送信
      send(accept_sock, html, strlen(html), 0);
    }else{  //ファイルがオープンできた場合(=200OKを返す)
      while ((ch = fgetc(fp))!=EOF){
        data[i]=ch;
        i++;
      }
      fclose(fp);
      data[i]='\0';
      //レスポンスヘッダの作成
      sprintf(html,
        "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n",len,type);
      //レスポンスヘッダの送信
      send(accept_sock, html, strlen(html), 0);

      //レスポンスボディ（HTMLデータ）の送信
      send(accept_sock, data, strlen(data), 0);
    }
  }else{  //html以外のファイル（＝バイナリファイル）を要求された場合
    //Content-Typeを調べる（とりあえずは.jpgと.pngの2種類）
    if(strstr(filename,".jpg")!=NULL)  strcpy(type,"image/jpeg");
    else if(strstr(filename,".png")!=NULL)  strcpy(type,"image/png");

    fp = fopen(filename, "rb");
    if(fp==NULL){  //ファイルがオープンできなかった場合（=処理の中断）
      printf("%s は存在しませんでした\n",filename);
      close(accept_sock);
      exit(EXIT_SUCCESS);
    }else{  //ファイルがオープンできた場合（=200OK）
      while(1){
        fread( data, 1, BUFSIZ, fp );
        i++;
        if(feof(fp)){
          fseek(fp, 0, SEEK_SET);
          break;
        }
      }
      //レスポンスヘッダの作成
      sprintf(html,
        "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n",BUFSIZ*i,type);
      //レスポンスヘッダの送信
      send(accept_sock, html, strlen(html), 0);
      while(1){
        len=fread( data, 1, BUFSIZ, fp );
        //レスポンスボディ（バイナリデータ）の送信
        send(accept_sock, data, len, 0);
        i=1;
        if(feof(fp)){
          fclose(fp);
          break;
        }
      }
    }
  }

  close(accept_sock);
  exit(EXIT_SUCCESS);
}

void signal_handler(int sig)  //SIGCHLDシグナルを受信したときのシグナルハンドラ
{
  int status, retval;
  //ゾンビプロセス（子プロセス）をすべて終了させる
  do{
    retval = waitpid(-1, &status, WNOHANG);
    if(retval > 0)
      printf("finished child process (%d)\n", retval);
  }while(retval > 0);
}

