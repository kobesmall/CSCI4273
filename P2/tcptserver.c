#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 8869 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/
//#define SIZE 1024;


void parseHTTP(int connfd,char *head,int n){
    printf("\n33333333333333333333333333333333333333333333333\n");
    char get[] ="GET";
    char data[1024];
    char pt[99];
    strncpy(pt,head,3);
    if(strncmp(get,head,3)!=0){
        printf("CALL UNAUTHORIZED METHOD ERROR\n");
        return;
    }
    else printf("GET METHOD CALLED\n");
      char *method = strtok(head," ");
      char *file = strtok(NULL, " ");                 //parse http request header (Method,filename,httpv)
      char *http = strtok(NULL, " ");
      char *end = strtok(NULL," ");

    const char *ext = strrchr(file+1, '.');
    if(!ext || ext == file+1) {
     ext = "";
     }
    ext = ext + 1;
    printf("EXTENSTION:%s\n",ext);


if (access(file+1, F_OK) != -1 ) {          ///accessing file  
      FILE *fp;
      fp =  fopen(file+1,"rb");
      printf("FILEPOINTER######%p####\n",fp);
      if(fp == NULL){
        printf("FILE PERMISSION ERROR\n");
        return;
      }



      fseek(fp, 0L, SEEK_END);              //file size
      unsigned long sz = ftell(fp);
      printf("FILEPOINTER######%p####\n",fp);

      char tet[500];
      char conlen[100];
      char filetype[100];
      strcpy(tet,"HTTP/1.1 200 OK\r\n");
      //snprintf(filetype,100,"Content-Type: .%s\r\n",ext);
      //strcat(tet,filetype);
      strcat(tet, "Content-Type: txt\r\n");
      snprintf(conlen, 100, "Content-Length: %lu\r\n", sz);
      printf("FILESIZE######%lu####\n",sz);
      printf("sizeof:%lu\n",sizeof(tet));
      printf("strlen:%lu\n",strlen(tet));
      strcat(tet, conlen);
      strcat(tet, "Connection: Keep-alive\r\n\r\n");
      send(connfd, tet, strlen(tet), 0);

      fseek(fp, 0, SEEK_SET);
      //printf("FILEPOINTER######%p####\n",fp);
          
      
      printf("FILE FOUND BOIS\n");
      //strncpy(data,"pHEADDDERtr",10);
      while(fgets(data ,sizeof(data), fp)!=NULL){
        printf("%s\n",data);
        send(connfd,data,strlen(data),0);
      }
      fclose(fp);
      char del[]="\r\n";
      send(connfd,del,strlen(del),0);

}
else{printf("NOUT FOUNNNDD\n");
      char _404[]= "HTTP/1.1 404 Not Found\r\n"
      "Content-type: text/html\r\n"
      "\n"
      "<html>\r\n"
      " <body>\r\n"
      "  <h1>404 File Not Found</h1>\r\n"
      "  <p>The requested File was not found on this server :( </p>\r\n"
      " </body>\r\n"
      "</html>\r\n\r\n";
      send(connfd, _404,strlen(_404),0);
      printf("SENT404\n");
      close(connfd);
      return;
}



    printf("%s\n",method);  
    printf("%s\n", file);
    printf("%s\n",http);
    printf("\n33333333333333333333333333333333333333333333333\n");
}









int main (int argc, char **argv)
{
 int listenfd, connfd, n;
 pid_t childpid;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;

 //Create a socket for the soclet
 //If sockfd<0 there was an error in the creation of the socket
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }


 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(SERV_PORT);

 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");

 for ( ; ; ) {

  clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

  printf("%s\n","Received request...");

  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process

  printf ("%s\n","Child created for dealing with client requests");

  //close listening socket
  close (listenfd);

  //char tet[100];
  //char txt[] = "<body> BOOMa\r\n";
  //char t[100];
  char h[100];
  while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
   // (n = recv(connfd, buf, MAXLINE,0)) > 0
   //n = recv(connfd, buf, MAXLINE,0);
   
   char *p = strstr(buf, "\r\n");
   char *w =buf;
   p = p+2;
   int i = p-w;
   strncpy(h,buf,i);

   //parseHTTP(connfd, h, n);
   /*
   printf("%s","String received from and resent to the client:");
   puts(buf);
   strcpy(tet,"HTTP/1.1 200 OK\r\n");
   strcat(tet, "Content-Type: .txt\r\n");
   snprintf(t, 90, "Content-Length: %lu\r\n", sizeof(txt));
   //printf("%s\n",t);
   strcat(tet, t);
   
   strcat(tet, "Connection: Keep-alive\r\n\r\n");
   //strcat(tet, txt);
   
   printf("header:%p\n",w);
   printf("%d",i);
   printf("header:%s\n",h);
   printf("\n\n%s\n\n",tet);
   //send(connfd, tet, n, 0);    
     */
   parseHTTP(connfd, h, n);
   //send(connfd, txt, n, 0); 

  }
  //printf("\n\n%sZZZZZ",buf);

   if (n < 0)
   printf("%s\n", "Read error");
  exit(0);
 }
 //close socket of the server
 close(connfd);
}
}