#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include "../messages/messages.h"

#define BUFFER_SIZE 2000

int n_servers;
int my_id;

int init_client_socket(char * address, char * port){
  int sfd;

  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if(getaddrinfo(address,port,&hints,&result) != 0){
    perror("Error on getaddrinfo");
    exit(1);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
     sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
     if (sfd == -1)
       continue;

     if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
       break;                  /* Success */

     close(sfd);
   }

   if (rp == NULL) {               /* No address succeeded */
     fprintf(stderr, "Could not connect\n");
     exit(EXIT_FAILURE);
   }

   freeaddrinfo(result);           /* No longer needed */

   return sfd;
}

int init_server_socket(int port){
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

int main(int argc, char * argv[]){

  if (argc<5){
    perror("Not enough arguments. Usage: server <port> <reg_ip> <reg_port> <dir>");
    exit(1);
  }

  char * port = argv[1];

  int server_fd = init_server_socket(atoi(argv[1])); //Socket where clients will connect

  int reg_fd = init_client_socket(argv[2],argv[3]); //Socket connected to the registry

  send_register(reg_fd,port);

  handle_r_ok(reg_fd,&n_servers,&my_id);
  printf("There are %d servers and I am server %d \n",n_servers, my_id);

  int start = handle_start(reg_fd);

  if(start){
    printf("Server %d: Ready to start\n",my_id);
    int req_id=0;
    if(my_id==0){
      req_id=1;
    }
    send_query(reg_fd,1);
    char ip[20];
    char port[4];

    memset(ip,'\0',10);
    memset(port,'\0',4);

    int rep = handle_q_ok(reg_fd,ip,port);

    if(rep == 1){
      printf("Reply to query: IP: %s PORT: %s\n",ip,port);
    }else if(rep == 0){
      printf("Reply to query: Q_ERR\n");
    }else{
      printf("Reply to query: NOT Q_OK(21) NOT Q_ERR(22)\n");
    }


    while(1){}
  }else{
    printf("Server %d: Start not received properly\n",my_id);
    while(1){}
  }
}
