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
#include <sys/sendfile.h>
#include "../messages/messages.h"

#define BUFFER_SIZE 2000

int n_servers;
int my_id;


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

void handle_client(int client_fd,int reg_fd, char * files_dir){

  printf("\nhandle_client.\n");

  char code;
  int read_size = read(client_fd,&code,1);

  printf("Code is: %d\n",code);

  switch (code) {
    case 12: //PUT
      handle_put(client_fd,reg_fd,n_servers,my_id,files_dir);
      break;
    case 13: //GET
      handle_get(client_fd,reg_fd,n_servers,my_id,files_dir);
      break;
    case 14: //DELETE
      handle_delete(client_fd,reg_fd,n_servers,my_id,files_dir);
      break;
  }


}

int main(int argc, char * argv[]){

  if (argc<5){
    perror("Not enough arguments. Usage: server <port> <reg_ip> <reg_port> <dir>");
    exit(1);
  }


  char files_dir[100];
  memset(files_dir,'\0',100);

  strcpy(files_dir,argv[4]);

  char * port = argv[1];

  int server_fd = init_server_socket(atoi(argv[1])); //Socket where clients will connect

  int reg_fd = init_client_socket(argv[2],argv[3]); //Socket connected to the registry

  send_register(reg_fd,port);

  handle_r_ok(reg_fd,&n_servers,&my_id);
  printf("There are %d servers and I am server %d \n",n_servers, my_id);

  int start = handle_start(reg_fd);

  if(start){
    printf("Server %d: Ready to start\n",my_id);

    int client_fd;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in client_addr;
    int pid;

    while(1){
      client_fd = accept(server_fd,(struct sockaddr *)&client_addr,&addrlen);
      if(client_fd<0){
        perror("Error after accept\n");
      }else{
        pid = fork();

        if(pid == 0){
          handle_client(client_fd,reg_fd,files_dir);
          exit(1);
        }else{
          close(client_fd);
        }
      }
    }
  }else{
    printf("Server %d: Start not received properly\n",my_id);
    while(1){}
  }
}
