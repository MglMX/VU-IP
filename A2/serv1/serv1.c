#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "wrapper.h"
#include <stdlib.h>
#include "keyvalue.h"

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

int attach_get_reply(char * reply,char * result,int s_pos){
  if(result==NULL){
    reply[s_pos]='n';
    s_pos+=1;
  }else{
    reply[s_pos]='f';
    s_pos+=1;
    strcat(&reply[s_pos],result);
    s_pos+=strlen(result);
    reply[s_pos]='\0';
    s_pos+=1;
  }
  return s_pos;
}

void handle_message(int client_fd,char * message){

  char  command[100];
  char reply[1000];
  memset(reply,'\0',1000);

  int stop=0;
  int s_pos=0;
  int reply_pos=0;

  char key[100];
  char value[100];
  char result[100];

  while(!stop){
    sprintf(command,"%s",&message[s_pos]);
    if(strlen(command)==0){
      stop=1;
    }else{
      if(command[0]=='p'){
        strcpy(key,&command[1]);
        s_pos+=strlen(key)+2;//p<key>\0_

        sprintf(value,"%s",&message[s_pos]);
        s_pos+=strlen(value)+1;

        put(key,value);
      }else if(command[0]=='g'){
        strcpy(key,&command[1]);
        s_pos+=strlen(key)+2; //g<key>\0_

        reply_pos=attach_get_reply(reply,get(key),reply_pos);
      }else{
        perror("Bad format command");
        stop=1;
      }
    }
  }

  //printer(reply,reply_pos);
  writen(client_fd,reply,reply_pos);

}

int main(int argc, char *argv[]){
    if(argc < 2){
      perror("Missing port number\n");
      exit(1);
    }

    int fd;

    fd = init_socket(atoi(argv[1])); //Change to argv[1]

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

            read_size = read(client_fd,message,BUFFER_SIZE);

            //printer(message,read_size);
            //printf("\n");

            handle_message(client_fd,message);

            memset(message,'\0',BUFFER_SIZE);

        }
    }


}
