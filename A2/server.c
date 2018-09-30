#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "wrapper.h"

#define BUFFER_SIZE 1000

void printer(char * m, int size){
  int i;

  for(i=0;i<size;i++){
    if(m[i]=='\0')
      printf("\\0");
    else
      printf("%c",m[i]);
  }
}

int init_socket(int port){
  struct sockaddr_in addr;

  int fd = socket(AF_INET,SOCK_STREAM,0); //Create a socket

  int enable = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) //Enabling reuseaddr
    perror("setsockopt(SO_REUSEADDR) failed");

  if(fd<0){
      perror("Error creating socket\n");
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);


  if(bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) <0){
      perror("Error binding socket\n");
  }

  if(listen(fd,5)<0){
    perror("Error listening\n");
  } //Make the server listen

  return fd;


}

int main(){
    int fd;

    fd = init_socket(5555); //Change to argv[1]

    int client_fd;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in client_addr;

    char message[BUFFER_SIZE];
    memset(message,'\0',BUFFER_SIZE);
    int read_size;

    while(1){
        client_fd = accept(fd,(struct sockaddr *)&client_addr,&addrlen); //Accept connections to this socket
        if(client_fd<0){
            perror("Error after accept\n");
        }else{
            printf("Aqui está el tio conectao\n");

            read_size = readn(client_fd,message,BUFFER_SIZE);
            printf("Message: %s\n",message);
            printer(message,read_size);
            printf("\n");


            char reply[1000];
            memset(reply,'\0',BUFFER_SIZE);
            strcpy(reply,"This is a reply moin froin");

            printf("Going to send\n");


            //writen(client_fd,reply,strlen("This is a reply moin froin")+1);
            writen(client_fd,reply,BUFFER_SIZE);

            printf("Enviao");
            fflush(stdout);
            
            /*
            read_size = readn(client_fd,message,BUFFER_SIZE);
            printf("Message: %s\n",message);
            printer(message,read_size);
            printf("\n");
            */

        }
    }


}
