/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h> 

#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}


typedef struct frame{
FILE *fp;                  //struct to breakup file and send multiple data grams and keep track of sequence number
FILE *endfp;                // file pointers to keep track of file progress in each data gram
int seqnum;
char data[200];

}frame;

int delete(int sockfd, struct sockaddr * clientaddr, socklen_t clientlen){
    char response[40];
    char suc[]= "SUCCESS";
    char fail[] ="FAIL";
    

    while(1){
        recvfrom(sockfd, &response, sizeof(response), 0, clientaddr, &clientlen);
        if(strncmp(response,suc,strlen(suc))==0){
            printf("DELETE SUCCESSFUL\n");
            return 0;
        }
        else if(strncmp(response,fail,strlen(fail))==0){
            printf("DELETION FAILURE\n");
            return 1;
        }

    }
    
    return 0;
}

//#####################################
//#####################################                                 //ls funciton
int ls_(int sockfd, struct sockaddr * clientaddr, socklen_t clientlen){
    char filename[270];
    char fin[]="FIN";
     
    while(1){
        recvfrom(sockfd, &filename, sizeof(filename), 0, clientaddr, &clientlen);           //loop over incoming date "filenames" and print
        if(strncmp(filename,fin,strlen(filename))==0){
            printf("\n\n");
            break;
        }
        printf("%s\n",filename);
        
    }
    
return 0;
}

//#####################################
//######################################                                //get function
int get(int sockfd, struct sockaddr * clientaddr, socklen_t clientlen){
    frame catch;
    catch.seqnum=0;
    char filename[270];
    char err[]= "**FILE NOT FOUND ERROR";
    /*struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
     fprintf(stderr,"TIMEOUT ERROR");*/

    recvfrom(sockfd, &filename, sizeof(filename), 0, clientaddr, &clientlen);
    if(strncmp(err,filename,strlen(err))==0){
    
        return 0;
    }
    //printf("filename: %s\n",filename);
    FILE *fp = fopen(filename,"wb");

    while(recvfrom(sockfd, &catch, sizeof(catch), 0, clientaddr, &clientlen)){
         

         if(catch.fp == catch.endfp){
            printf("FILE TRANSFER COMPLETE\n");                     //compare file pointers of frames to check when end of file has been reached 
            fclose(fp);
            break;
        }
        //printf("%d\n",catch.seqnum);
        //printf("FRAME:{%s}\n",catch.data);
        
        //printf("fp:%p\n",catch.fp);
        //printf("ENDP:%p\n",catch.endfp);
       
        //fwrite(catch.data,sizeof(catch.data),1,fp);
        //fputs(catch.data,fp);
       // fprintf(fp,"%s",catch.data);
        
    }
   //h }
    return 0;
}
/*#############################################################
#############################################################*/                         //put function
int put(int sockfd,char *buf, struct sockaddr * clientaddr, socklen_t clientlen){
 char ans[100];
 char *fil = buf + 4;
 char *tf  = fil + strlen(fil)-1;                                                       //strip filename from <command filename> from client
 char pil[] ="\0";
 strcpy(tf,pil);
 frame toss;
 toss.seqnum=0;



//printf("%s\n",fil);
//printf("%zu\n",sizeof(fil));

 //printf("%lu\n",strlen(fil));
if (access(fil, F_OK) != -1 ) { 
    
  sendto(sockfd, fil, strlen(fil)+1, 0, clientaddr, clientlen);
      //printf("Sending File: %s\n",fil);
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
      toss.fp = toss.endfp;
      sendto(sockfd, &toss, sizeof(toss), 0, clientaddr, clientlen);
      recvfrom(sockfd, ans, sizeof(ans), 0, clientaddr, &clientlen);
      printf(" %s\n",ans);
      //printf("end file pointer: %p\n",toss.endfp);
      fclose(toss.fp);

      //fclose(toss.endfp);
      
      //sendto(sockfd, pil,sizeof(pil),0, clientaddr, clientlen);
      //printf("finished\n");
      //printf("size of frame %lu\n",sizeof(toss));  
}
else{
  printf("FILE NOT FOUND: %s\n",fil);
  char err[]= "**FILE NOT FOUND ERROR";
  int n = sendto(sockfd, err, strlen(err), 0, clientaddr, clientlen);
               if (n < 0) 
                  error("ERROR in sendto");
    return 0;
}
 
    
  return 0;
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    //int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

char ex[] = "\n###EXIT CALLED###";
//char exit[]="exit";
char getc[] = "get ";
char snd[] = "put" ;                                    //menu control check userinput and jump into appropriate command function
char del[] = "delete ";
char ls[] = "ls";
int hld = strlen(ex);

while((strncmp(buf,ex,hld)!=0)){
//printf("%lu\n",strlen(buf));
//printf("%lu\n",strlen(ex));

    /* get a message from the user */
    bzero(buf, BUFSIZE);
    printf("COMMAND: ");
    fgets(buf, BUFSIZE, stdin);
    /* send the message to the server */
    socklen_t serverlen = sizeof(serveraddr);
    
    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    
    /* print the server's reply */
    if(strncmp(buf,getc,strlen(getc))==0){
        printf("GET CALLED\n");
        get(sockfd,(struct sockaddr*)&serveraddr,serverlen);
    }

    else if(strncmp(buf,snd,strlen(snd))==0){
        printf("PUT CALLED\n");
        put(sockfd,buf,(struct sockaddr*)&serveraddr,serverlen);
    }

    else if(strncmp(buf,ls,strlen(ls))==0 ){
        printf("LS CALLED\n\n");
        ls_(sockfd,(struct sockaddr*)&serveraddr,serverlen);
    }
    else if(strncmp(buf,del,strlen(del))==0){
        printf("DELETE CALLED\n");
        delete(sockfd,(struct sockaddr*)&serveraddr,serverlen);
    }
    else{

    

    n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr*) &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");
    printf("server: %s\n", buf);
    
    }
}
    return 0;
}
