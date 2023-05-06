#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>                         //Built off from echo server resource given https://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html

#define MAXLINE 3000 /*max text line length*/
#define SERV_PORT 8869 /*port*/
#define LISTENQ 15 /*maximum number of client connections*/
//#define SIZE 1024;



void _400ERROR(int connfd,char * http){
  char _400[300];
  snprintf(_400,300,"%s 400 Bad Request\r\n"                           //400 Bad Request

      "Content-type: text/html\r\n"
      "\n"
      "<html>\r\n"
      " <body>\r\n"
      "  <h1>400 Bad Request</h1>\r\n"
      "  <p>The request could not be parsed or is malformed </p>\r\n"
      " </body>\r\n"
      "</html>\r\n",http);
              send(connfd, _400,strlen(_400),0);
              printf("%s\n",_400);
              close(connfd);
}

void _403ERROR(int connfd,char * http){
  char _403[300];
  snprintf(_403,300,"%s 403 Forbidden\r\n"                           //505 HTTP Version Not Supported ERROR
              "Content-type: text/html\r\n"
              "\n"
              "<html>\r\n"
              " <body>\r\n"
              "  <h1>403 Forbidden </h1>\r\n"
              "  <p>The requested file can not be accessed due to a file permission issue</p>\r\n"
              " </body>\r\n"
              "</html>\r\n\r\n",http);
              send(connfd, _403,strlen(_403),0);
              printf("%s\n",_403);
              close(connfd);
}

void _404ERROR(int connfd,char * http){
  char _404[300];
  snprintf(_404,300,"%s 404 Not Found\r\n"                           //404 NOT FOUND ERROR
      "Content-type: text/html\r\n"
      "\n"
      "<html>\r\n"
      " <body>\r\n"
      "  <h1>404 File Not Found</h1>\r\n"
      "  <p>The requested file can not be found in the document tree</p>\r\n"
      " </body>\r\n"
      "</html>\r\n",http);
              send(connfd, _404,strlen(_404),0);
              printf("%s\n",_404);
              close(connfd);
}

void _405ERROR(int connfd,char * http){
  char _405[300];
  snprintf(_405,300,"%s 405 Method Not Allowed\r\n"                           //405 Method Not Allowed ERROR
              "Content-type: text/html\r\n"
              "\n"
              "<html>\r\n"
              " <body>\r\n"
              "  <h1>405 Method Not Allowed </h1>\r\n"
              "  <p>A method other than GET was requested</p>\r\n"
              " </body>\r\n"
              "</html>\r\n\r\n",http);
              send(connfd, _405,strlen(_405),0);
              printf("%s\n",_405);
              close(connfd);
}


void _505ERROR(int connfd,char * http){
  char _505[300];
  snprintf(_505,300,"%s 505 HTTP Version Not Supported\r\n"                           //505 HTTP Version Not Supported ERROR
              "Content-type: text/html\r\n"
              "\n"
              "<html>\r\n"
              " <body>\r\n"
              "  <h1>505 HTTP Version Not Supported </h1>\r\n"
              "  <p>An HTTP version other than 1.0 or 1.1 was requested</p>\r\n"
              " </body>\r\n"
              "</html>\r\n\r\n",http);
              send(connfd, _505,strlen(_505),0);
              printf("%s\n",_505);
              close(connfd);
}

 char *checkhtml(char*file ){
    //char html[]="index.html";                             //Handles '/' request case 
    
    static char bam[100];
    snprintf(bam,100,"%sindex.html",file);
    static char sut[100];
    snprintf(sut,100,"%sindex.htm",file);

    //printf("********%s\n",bam);
    //printf("*******%s\n",sut);
    if (access(bam+1, F_OK) != -1 ) {  
    return bam;
    }
    else if(access(sut+1, F_OK) != -1 ){
        return sut;
    }
    else{
       
        return "error";
    }
    return "error";
}




void parseHTTP(int connfd,char *head){
   // printf("\n33333333333333333333333333333333333333333333333\n");
    
    char get[] ="GET";
    char _1[] ="HTTP/1.1";
    char _0[] ="HTTP/1.0";
    char *rfile = (char*) malloc(100*sizeof(char));
  
    char data[800];
   
    char *method = strtok(head," ");
    char *file = strtok(NULL, " ");                 //parse http request header (Method,filename,httpv)
    char *http = strtok(NULL, " ");
    
    if(strncmp(_1,http,strlen(_1))==0){ http = _1; }                                //fixing format for http version
    else if(strncmp(_0,http,strlen(_1))==0){ http = _0; }
    else _505ERROR(connfd,_1);
   
    snprintf(rfile,100,"/www%s",file);
    
    char *pos = rfile + strlen(rfile) -1;                                             //check for / request
    if(strncmp(pos,"/",1) == 0){  strncpy(rfile,checkhtml(rfile),100);    }  
    if(strstr(rfile,"//") != NULL ){ _400ERROR(connfd,http); }
    if(strncmp(rfile,_1,strlen(_1))==0 || strncmp(method ,_1,strlen(_1))==0){   _400ERROR(connfd,_1);}
    


    DIR* dir = opendir(rfile+1);
  if (dir) {
    /* Directory exists. */
    static char tan[100];
    snprintf(tan,100,"/www%s/index.html",file);
    static char bk[100];
    snprintf(bk,100,"/www%s/index.htm",file);             // director request case html htm
    if (access(tan+1, F_OK) != -1 ) {  
    strncpy(rfile,tan,100); 
    }
    else if(access(bk+1, F_OK) != -1 ){
        strncpy(rfile,bk,100); 
    }
    else {strncpy(rfile,"err",3);}
    closedir(dir);
} 


    
    if(strncmp(get,head,3)!=0){
        _405ERROR(connfd,http);
        free(rfile);
        return;
    }
    

    const char *ext = strrchr(rfile+1, '.');
    if(!ext || ext == file+1) {
     ext = "";
     }
    ext = ext + 1;
   

//snprintf(rut,100,"/www%s",file);

if (access(rfile+1, F_OK) != -1 ) {          ///accessing file  
      FILE *fp;
      fp =  fopen(rfile+1,"rb");
      

      if(fp == NULL){
        _403ERROR(connfd,http);
        free(rfile);
        return;
      }



      fseek(fp, 0L, SEEK_END);              //file size
      unsigned long sz = ftell(fp);
      

      char tet[300];
      char conlen[100];
      char filetype[100];
      char httpv[25];

      //strcpy(tet,"HTTP/1.1 200 OK\r\n");
      snprintf(httpv,25,"%s 200 OK\r\n",http);
      strcat(tet,httpv);
      snprintf(filetype,100,"Content-Type: .%s\r\n",ext);
      strcat(tet,filetype);
      
      snprintf(conlen, 100, "Content-Length: %lu\r\n", sz);
      //printf("FILESIZE######%lu####\n",sz);
      //printf("sizeof:%lu\n",sizeof(tet));
      //printf("strlen:%lu\n",strlen(tet));
      strcat(tet, conlen);
      strcat(tet, "Connection: Keep-alive\r\n\r\n");
      printf("%s\n",tet);
      send(connfd, tet, strlen(tet), 0);

      fseek(fp, 0, SEEK_SET);
      //printf("FILEPOINTER######%p####\n",fp);
          
      
      //printf("FILE FOUND BOIS\n");
      int i= 0;
      size_t nbytes = 0;
      
      while((nbytes = fread(data ,1, 800, fp)) >0){
        //printf("%s\n",data);
        //send(connfd,data,strlen(data),0);
        write(connfd,data,sizeof(data));                        //write data to socket 
        i++;
        //&data=&data+i;
      }
      fclose(fp);
      char del[]="\r\n";
      write(connfd,del,sizeof(del));

}
else{
        _404ERROR(connfd,http);                         //file not found error 404
        free(rfile);
      return;   
}

    free(rfile);
    close(connfd);
    //printf("%s\n",method);  
    //printf("%s\n", file);
    //printf("%s\n",http);
    //printf("\n33333333333333333333333333333333333333333333333\n");
}









int main (int argc, char **argv)
{
 int portno;
 int listenfd, connfd, n;
 pid_t childpid;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;


 /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);


 //Create a socket for the soclet
 //If sockfd<0 there was an error in the creation of the socket
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }


 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(portno);

 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

 printf("# Running your server with a port # of %d\n",portno);

 for ( ; ; ) {

  clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

  printf("%s\n","Received request...");

  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process

  printf ("%s\n","Child created for dealing with client requests");

  //close listening socket
  close (listenfd);

  char h[300];
  while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {

   char *p = strstr(buf, "\r\n");
   char *w =buf;
   p = p+2;                                             //Stripping just first line of http reqest header
   int i = p-w;
   strncpy(h,buf,i);

   parseHTTP(connfd, h);

  }

   if (n < 0)
   //printf("%s\n", "Read error");
   close(connfd);
  exit(0);
 }
 //close socket of the server
 close(connfd);
}
}
