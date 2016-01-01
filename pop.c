#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//グローバル変数たち
static char *p;
static char filename[100][100];
const char base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int j;

//BASE64をデコードする関数
char *_strchr(const char *s, char c){
    while (*s != '\0') {
        if (*s == c) return (char *)s;
        s++;
    }
    return NULL;
}
char _decode(char c1, char c2, int no){
    char b1, b2, r;
    b1 = _strchr(base64, c1) - base64;
    b2 = _strchr(base64, c2) - base64;
    switch (no) {
        case 0:
            r = (b1 << 2) | (b2 >> 4);
            break;
        case 1:
            r = (b1 << 4) | (b2 >> 2);
            break;
        case 2:
            r = (b1 << 6) | b2;
            break;
    }
    return r;
}

//ファイル名を取得する関数
void getfilename(){
  int i=0;
  while(1){
    printf("%c",*p);
    p++;
    if(*p=='e'&&*(p+1)=='n'&&*(p+2)=='a'&&*(p+3)=='m'&&*(p+4)=='e'&&*(p+5)=='='&&*(p+6)=='"')  break;
  }
  while(1){
    printf("%c",*p);
    p++;
    if(*p=='"'){
      i=0;
      while(1){
        printf("%c",*p);
        p++;
        if(*p=='"')  break;
        filename[j][i]=*p;
        i++;
      }
      filename[j][i]='\0';
    }
    if(*(p-4)=='\r'&&*(p-3)=='\n'&&*(p-2)=='\r'&&*(p-1)=='\n')  break;
  }
  j++;
}

//エラーメッセージ
void err_func(char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  unsigned int ch,sockfd,len,i,n,flg,tmp;
  char b[3], c[4];
  char buf[BUFSIZ];
  char user[100],pass[100];
  struct sockaddr_in serv;
  unsigned short port=110;
  char str[20000];
  FILE *fp = NULL;
  FILE *fi = NULL, *fo = NULL;

  //実行ファイル名とPOP3サーバアドレスの2つの要素が入力されているかチェック
  if(argc != 2)
  {
    printf("usage: progname serv_ip serv_port\n");
    exit(EXIT_FAILURE);
  }

  //socketシステムコール
  if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    err_func("socket");

  serv.sin_family = PF_INET;
  serv.sin_port = htons(port);
  inet_aton(argv[1], &(serv.sin_addr));

  //connectシステムコール
  if(connect(sockfd, (struct sockaddr *)&serv, sizeof(struct sockaddr_in)) < 0)
    err_func("connect");

  //connectが完了したことを伝えるメッセージを受信
  len = recv(sockfd, buf, BUFSIZ, 0);
  buf[len] = '\0';
  printf("%s\n", buf);

  do{
    len = recv(sockfd, buf, BUFSIZ, 0);  //Continuation（2バイト）

    strcpy(user,"USER ");
    strcpy(pass,"PASS ");

    //ユーザ名の送信
    printf("USER: ");
    fgets(buf, BUFSIZ, stdin);
    strcat(user,buf);
    user[strlen(user)-1] = '\0';
    strcat(user, "\r\n");  //CRLFを挿入
    len = strlen(user);
    len = send(sockfd, user, len, 0);
    len = recv(sockfd, buf, BUFSIZ, 0);  //+OK or -ERR
    buf[len] = '\0';
    printf("%s",buf);

    //パスワードの送信
    printf("Password: ");
    fgets(buf, BUFSIZ, stdin);
    strcat(pass,buf);
    pass[strlen(pass)-1] = '\0';
    strcat(pass, "\r\n");  //CRLFを挿入
    len = strlen(pass);
    len = send(sockfd, pass, len, 0);
    len = recv(sockfd, buf, BUFSIZ, 0);  //+OK or -ERR
    buf[len] = '\0';
    printf("%s\n", buf);

  }while(buf[0]!='+');  //ログインに成功するまで繰り返す

  len = recv(sockfd, buf, BUFSIZ, 0);  //Continuation（2バイト）

  //各コマンドの実行
  while(1){
    printf("> ");
    fgets(buf, BUFSIZ, stdin);
    buf[strlen(buf)-1] = '\0';
    strcat(buf, "\r\n");  //CRLFを挿入
    len = strlen(buf);
    if(strncmp(buf, "quit\r\n", 6) == 0){  //quitが入力された場合は処理を終了
      len = send(sockfd, buf, len, 0);
      len = recv(sockfd, buf, BUFSIZ, 0);
      buf[len] = '\0';
      printf("%s", buf);
      break;
    }

    len = send(sockfd, buf, len, 0);

    while(1){
      len = recv(sockfd, buf, BUFSIZ-1, 0);
      buf[len] = '\0';
      p=strstr(buf,"..");
      if(p!=NULL)  *(p+1)=' ';
      printf("%s", buf);
      p=strstr(buf,"filename=");
      if(p!=NULL)  break;
      else if(buf[len-1]=='\n'&&buf[len-2]=='\r'&&buf[len-3]=='.')  break;
    }
    if(p!=NULL){  //ファイルが添付されている場合の処理（仮）
      p=buf;
      while(1){
        getfilename();  //ファイル名取得
        flg=0;
        tmp=0;
        fp = fopen( "read.txt", "w" );
        while(1){  //添付ファイルのデータ部分を read.txt に書き込み
          while(1){
            printf("%c", *p);
            fputc(*p,fp);
            if(*(p+1)=='\r'&&*(p+2)=='\n'){  //文末の\r\n判定
              p++;
              printf("%c", *p);
              p++;
              printf("%c", *p);
              if(*p=='\0'){
                len = recv(sockfd, buf, BUFSIZ-1, 0);
                buf[len] = '\0';
                p=buf;
              }
              if(*(p+1)=='\r'&&*(p+2)=='\n'){  //1つの区切り判定
                p++;
                printf("%c", *p);
                p++;
                printf("%c", *p);
                flg=1;
                break;
              }
              else if(*(p+1)=='\r'&&*(p+2)=='\0'){  //1つの区切り判定　例外
                p++;
                printf("%c", *p);
                len = recv(sockfd, buf, BUFSIZ-1, 0);
                buf[len] = '\0';
                p=buf;
                printf("%c",*p);
                flg=1;
                break;
              }
              else if(*(p+1)=='.'&&*(p+2)=='\r'&&*(p+3)=='\n'){  //終わり判定
                p++;
                printf("%c", *p);
                p++;
                printf("%c", *p);
                p++;
                printf("%c", *p);
                flg=1;
                tmp=1;
                break;
              }
              else if(*(p+1)=='.'&&*(p+2)=='\r'&&*(p+3)=='\0'){  //終わり判定　例外1
                p++;
                printf("%c", *p);
                p++;
                printf("%c", *p);
                len = recv(sockfd, buf, BUFSIZ-1, 0);
                buf[len] = '\0';
                p=buf;
                printf("%c",*p);
                flg=1;
                tmp=1;
                break;
              }
              else if(*(p+1)=='.'&&*(p+2)=='\0'){  //終わり判定　例外2
                p++;
                printf("%c", *p);
                len = recv(sockfd, buf, BUFSIZ-1, 0);
                buf[len] = '\0';
                p=buf;
                printf("%c",*p);
                p++;
                printf("%c",*p);
                flg=1;
                tmp=1;
                break;
              }
            }
            else if(*(p+1)=='\r'&&*(p+2)=='\0'){  //文末の\r\n判定　例外
              p++;
              printf("%c",*p);
              len = recv(sockfd, buf, BUFSIZ-1, 0);
              buf[len] = '\0';
              p=buf;
              printf("%c",*p);
              p++;
              continue;
            }
            p++;
            if(*p=='\0')  break;
          }
          if(flg==1)  break;
          else{
            len = recv(sockfd, buf, BUFSIZ-1, 0);  //添付ファイルのデータが分割されていた場合の受信
            buf[len] = '\0';
            p=buf;
            if(*p=='\r'){  //例外
              printf("%c",*p);
              p++;
            }
          }
        }
        fclose(fp);
        fi = fopen("read.txt", "rt");
           fo = fopen(filename[j-1], "wb");
        i = 0;
           while ((ch = fgetc(fi))!=EOF) {  //BASE64デコード
               if (ch != '\n') {
                 c[i] = ch;
                  i++;
                    if (i == 4) {
                           b[0] = _decode(c[0], c[1], 0);
                           b[1] = _decode(c[1], c[2], 1);
                           b[2] = _decode(c[2], c[3], 2);
                           n = 3;
                           if (c[3] == '=') n = 2;
                           if (c[2] == '=') n = 1;
                           fwrite(b, 1, n, fo);
                      i = 0;
                    }
                }
        }
        if (i > 0)  printf("入力データの形式が正しくない\n");
            fclose(fi);
            fclose(fo);
        if(tmp==0){
          //複数ファイルが添付されていた場合、次のfilenameのところまでポインタを移動しつつ表示
          while((*p!='.')&&
          (*p!='e'&&*(p+1)!='n'&&*(p+2)!='a'&&*(p+3)!='m'&&*(p+4)!='e'&&*(p+5)!='='&&*(p+6)!='"')){
            p++;
            printf("%c",*p);
          }
          if(*p=='.')  tmp=1;
          else p++;
        }
        if(tmp==1)  break;
      }
      printf("\n");
      for(i=0;i<j;i++)  //保存した添付ファイル名の表示
        printf("*****添付ファイルを %s に出力しました*****\n",filename[i]);
      j=0;
    }
  }
  close(sockfd);  //socketクローズ
  return 0;
}
