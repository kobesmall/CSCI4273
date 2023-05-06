/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

typedef struct frame{               //struct to breakup file and send multiple data grams and keep track of sequence number
FILE *fp;                           // file pointers to keep track of file progress in each data gram
FILE *endfp;
int seqnum ;
char data[200];

}frame;

int delete(int sockfd,char *buf, struct sockaddr * clientaddr, socklen_t clientlen){      //delete funciton
  char *fil = buf + 7;
  char *tf  = fil + strlen(fil)-1;
  char pil[] ="\0";
  char suc[] ="SUCCESS";
  char fail[] ="FAIL";
  char sv[] ="server.c";
  int a;
 
  strcpy(tf,pil);
  if(strncmp(fil,sv,strlen(sv))==0){
     sendto(sockfd,fail, sizeof(fail), 0, clientaddr, clientlen);
  }
  a = remove(fil);
  if(a==0){
  sendto(sockfd,suc, sizeof(suc), 0, clientaddr, clientlen);                    //test success or fail based on server response
  }
  if(a!=0){
    sendto(sockfd,fail, sizeof(fail), 0, clientaddr, clientlen);
  }

  printf("filename: %s\n",fil);
  return 0;
}
//#########################################################         //ls function
//########################################################
void ls_(int sockfd, struct sockaddr * clientaddr, socklen_t clientlen){
  char fin[]="FIN";       //making pointer to pwd
  DIR *mydir;
  struct dirent *myfile;
   mydir = opendir("./");
    while((myfile = readdir(mydir)) != NULL)
    {
            
    sendto(sockfd, myfile->d_name, sizeof(myfile->d_name), 0, clientaddr, clientlen);       //list over file in pwd and send to client
        
    }
    closedir(mydir);
  sendto(sockfd, fin, sizeof(fin), 0, clientaddr, clientlen);

}
//################################################
//################################################

//###getf

int getf(int sockfd,char *buf, struct sockaddr * clientaddr, socklen_t clientlen){

 // sendto();
 //char filen[]="Finished";
 char *fil = buf + 4;
 char *tf  = fil + strlen(fil)-1;               //grab filename from <command filename>
 char pil[] ="\0";
 
 
 strcpy(tf,pil);
 
 frame toss;
 toss.seqnum=0;

//printf("%s\n",fil);
//printf("%zu\n",sizeof(fil));

 printf("%lu\n",strlen(fil));
if (access(fil, F_OK) != -1 ) { 
    
  sendto(sockfd, fil, strlen(fil)+1, 0, clientaddr, clientlen);
      printf("Sending File: %s\n",fil);
      toss.fp = fopen(fil,"rb");
      toss.endfp = fopen(fil,"rb");
      fseek(toss.endfp, 0L, SEEK_END);


      while (fgets(toss.data,sizeof(toss.data),toss.fp)!= NULL){
      toss.seqnum++;  
      sendto(sockfd, &toss, sizeof(toss), 0, clientaddr, clientlen);
      //printf("chunk sent...%lu\n",sizeof(toss.data));
      }

      toss.seqnum++;
      strncpy(toss.data,"END OF FILE",sizeof(toss.data));
      toss.fp = toss.endfp;                                                   //update filepointer in struct to show client end of file has been reached
      sendto(sockfd, &toss, sizeof(toss), 0, clientaddr, clientlen);
      //printf("file pointer: %p\n",toss.fp);
      //printf("end file pointer: %p\n",toss.endfp);
      fclose(toss.fp);

      //fclose(toss.endfp);
      
      //sendto(sockfd, pil,sizeof(pil),0, clientaddr, clientlen);
      printf("finished\n");
      //printf("size of frame %lu\n",sizeof(toss));  
}
else{
  printf("File not found: %s\n",fil);
  char err[]= "**FILE NOT FOUND ERROR";
  int n = sendto(sockfd, err, strlen(err), 0, clientaddr, clientlen);
               if (n < 0) 
                  error("ERROR in sendto");
    return 0;
}
 //printf("file: %s\n",fil);

  return 0;
}
/*#####################################################
#####################################################*/     //put function 

int putf(int sockfd, struct sockaddr * clientaddr, socklen_t clientlen){
  frame pcatch;
    pcatch.seqnum=0;
    char filename[270];
    char err[]= "**FILE NOT FOUND ERROR";
    char fin[]="FILE TRANSFER COMPLETE";
    
    recvfrom(sockfd, &filename, sizeof(filename), 0, clientaddr, &clientlen);
    
    if(strncmp(err,filename,strlen(err))==0){
        printf("%s\n",err);
        return 0;
    }
    //printf("filename: %s\n",filename);
    FILE *fp = fopen(filename,"wb");

    while(recvfrom(sockfd, &pcatch, sizeof(pcatch), 0, clientaddr, &clientlen)){
         

         if(pcatch.fp == pcatch.endfp){
            printf("FILE TRANSFER COMPLETE\n");
            fclose(fp);
            sendto(sockfd, fin, strlen(fin), 0, clientaddr, clientlen);
            break;
        }
        //printf("%d\n",pcatch.seqnum);
        //printf("FRAME:{%s}\n",pcatch.data);
        
        //printf("fp:%p\n",catch.fp);
        //printf("ENDP:%p\n",catch.endfp);
       
        //fwrite(catch.data,sizeof(catch.data),1,fp);
        //fputs(catch.data,fp);
        fprintf(fp,"%s",pcatch.data);
        
    } 
    return 0;
}


int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0){
    error("ERROR on binding");}
    printf("##SERVING ON PORT: %i...........\n",portno);

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    unsigned int bld = clientlen;
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &bld);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
     int hld = strlen(buf);
    printf("server received %d/%d bytes: %s\n", hld, n, buf);
    
  
    char cu[] = "\n###EXIT CALLED###\n##YOU HAVE LEFT THE SERVER##\n#GOODBYE!#\n";                //menu control compare client data to different menu commands if non fit command not found
    char ex[] = "exit";
    char ls[] = "ls";
    char get[] = "get ";
    char put[] = "put ";
    char del[] = "delete ";
    //printf("%d\n",strncmp(buf,get,hld-1));
    //printf("%lu\n",strlen(get));
      if(strncmp(ex,buf,strlen(ex))==0){
               n = sendto(sockfd, cu, strlen(cu), 0, (struct sockaddr *) &clientaddr, clientlen);
               if (n < 0) 
                  error("ERROR in sendto");
      }
      else if (strncmp(buf,get,strlen(get))==0){
        printf("getf called\n");
        getf(sockfd,buf,(struct sockaddr *) &clientaddr,clientlen);

      }
      else if (strncmp(buf,put,strlen(put))==0){
        printf("put called\n");
        putf(sockfd,(struct sockaddr *) &clientaddr,clientlen);
      }
      else if(strncmp(buf,ls,strlen(ls))==0){
        printf("ls called\n");
        ls_(sockfd,(struct sockaddr *) &clientaddr,clientlen);
      }
      else if (strncmp(buf,del,strlen(del))==0){
        printf("delete called\n");
        delete(sockfd,buf,(struct sockaddr *) &clientaddr,clientlen);
      }
      else{
        char cmf[] ="COMMAND NOT FOUND\n";
    n = sendto(sockfd, cmf, strlen(cmf), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
  }
   
  }


    /* 
     * sendto: echo the input back to the client 
     */
    /*
    char cmf[] ="COMMAND NOT FOUND\n";
    n = sendto(sockfd, cmf, strlen(cmf), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
  } */
}
