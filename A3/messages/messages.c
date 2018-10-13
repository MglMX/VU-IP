#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "messages.h"
#include <errno.h>

#define BUFFER_SIZE 2000

/*
* Wrapper function from
UNIX Network Programming: Sockets Introduction
By Andrew M. Rudoff, Bill Fenner, W. Richard Steven
*/
ssize_t writen(int fd, const void *vptr, size_t n)
{
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;

  int errno;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ( (nwritten = write(fd, ptr, nleft)) <= 0 ) {
    if (errno == EINTR)
      nwritten = 0; /* and call write() again */
    else
      return -1;
    /* error */
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}

void printer(char * m, int size){
  int i;

  for(i=0;i<size;i++){
    if(i == 0){
      printf("%d",m[i]);
    }else{
      if(m[i]=='\0')
        printf("\\0");
      else
        printf("%c",m[i]);
    }

  }
  printf("\n");
}

/////////////////////////////////////////////////////////////
// SERVER
/////////////////////////////////////////////////////////////

void handle_r_ok(int reg_fd,int * n_servers,int * my_id){
  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);
  int read_size = read(reg_fd,message,BUFFER_SIZE);
  printer(message,read_size);

  *n_servers = message[1];
  *my_id = message[2];

  printf("There are %d servers and I am server %d \n",*n_servers, *my_id);
}

int send_query(int reg_fd, int id){
  char message[3];
  memset(message,'\0',strlen(message));

  message[0]=11; //QUERY code
  message[1]=id; //ID of server to QUERY

  writen(reg_fd,message,strlen(message));
}

int send_register(int reg_fd, char * port){
  char message[6];
  memset(message,'\0',strlen(message));

  message[0]=10; //REGISTER code

  strcat(&message[1],port);

  return writen(reg_fd,message,strlen(message));

}

/////////////////////////////////////////////////////////////
// REGISTRY
/////////////////////////////////////////////////////////////

/**
* The method checks the ip and port of server with id id in the servers array and returns it to the server that requested it
**/
void handle_query(int server_fd, int id, struct server_info * servers){

}

/**
* Receives the listening port of the server connected. Stores it in the servers array. Replies to the server with 20:n_servers:ID
*/
void handle_register(int id,char * message, int size,int n_servers,struct server_info *servers){
  printer(message,size);

  char port[10];
  strcpy(port,&message[1]); //Copying the port from the message skypping the code

  printf("Server %d connected with port: %s\n",id,port);

  //strcpy(servers[id].port,port);

  char reply[10];
  memset(reply,'\0',strlen(reply));
  reply[0]=20;
  reply[1]=n_servers;
  reply[2]=id;

  printf("reply[0]=%d,reply[1]=%d,reply[2]=%d \n fd: %d\n",reply[0],reply[1],reply[2],servers[id].fd);
  printf("reply: ");
  printer(reply,strlen(reply));
  writen(servers[id].fd,reply,strlen(reply));
  printf("Message sent\n");
}
