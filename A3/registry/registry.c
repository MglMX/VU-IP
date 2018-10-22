#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../messages/messages.h"

#define BUFFER_SIZE 2000 //Check in meesages.c if change


int n_servers;
struct server_info *servers; //Initialized in registry.c

void print_servers(){
  int i;
  for(i=0;i<n_servers;i++){
    printf("servers[%d]:{\n id: %d\n fd: %d\n ip: %s\n port:%s\n}\n",i,servers[i].id,servers[i].fd,servers[i].ip,servers[i].port);
  }
}
void handle_server(int id){
    printf("Handling server %d\n",id);
    char message[BUFFER_SIZE];
    int server_fd = servers[id].fd;

    int read_size = read(server_fd,message,BUFFER_SIZE);

    printer(message,read_size);

    char code = message[0];

    printf("Code received: %d\n",code);

    switch (code) {
      case 10: //REGISTER
        handle_register(id,message,read_size,n_servers,servers);
        break;
      case 11: //QUERY
        printf("Case: 11 QUERY\n");
        printf("Case: id: %d\n",message[1]);
        handle_query(server_fd,message,n_servers,servers);
        break;

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

//Wait for n_servers to connect and sends them their ID
void receive_servers(int n_servers, int reg_fd){
  socklen_t addrlen = sizeof(struct sockaddr_in);
  struct sockaddr_in server_addr;

  int pid;
  int i;
  for(i=0;i<n_servers;i++){
    printf("Waiting for accept\n");
    int server_fd = accept(reg_fd,(struct sockaddr *)&server_addr,&addrlen);
    printf("Server connected. IP: %s PORT: %d \n ID: %d fd: %d\n",inet_ntoa(server_addr.sin_addr),server_addr.sin_port,i,server_fd);
    servers[i].id = i;
    servers[i].fd = server_fd;
    strcpy(servers[i].ip,inet_ntoa(server_addr.sin_addr));
    //servers[i].port //Initialized in handle_server
    if(server_fd<0){
      perror("Error accepting connection\n");
      exit(1);
    }
    handle_server(i);
  }
  printf("Done receiving servers\n");
}

int main(int argc, char * argv[]){
  if(argc<3){
    perror("Not enoguh arguments. Usage: registry <port> <n_servers>");
    exit(1);
  }

  n_servers = atoi(argv[2]);

  servers = malloc(sizeof(struct server_info)*n_servers); //Allocating memory for n_servers
  int reg_fd = init_socket(atoi(argv[1]));

  receive_servers(n_servers,reg_fd);

  printf("------------------------\n");
  char ip[20];
  //print_servers();
  printf("Servers[%d].ip = %s(%zu)\n",0,servers[0].ip,strlen(servers[0].ip));
  strcpy(ip,servers[0].ip);
  printf("copied ip: %s\n",ip);
  printf("------------------------\n");

  printf("All servers connected\n");
  send_start(n_servers,servers);
  //Do something after all servers are connected
  printf("Sent start to all servers\n\n");

  int i,pid;
  for(i=0;i<n_servers;i++){
    pid = fork();
    if(pid == 0){
      printf("I am child %d\n",i);
      //print_servers();
      while(1){
        handle_server(i);
      }
    }
  }

  //close(reg_fd);
  return 0;
}
