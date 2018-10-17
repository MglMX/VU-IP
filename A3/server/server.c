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

void handle_client(int client_fd){

  printf("\nhandle_client.\n");

  unsigned char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  FILE *file_stream;

  file_stream = fopen("test.zip","wb");

  int read_size;

  unsigned long hash;

  do{

    //read_size = sendfile(fileno(file_stream),client_fd,NULL,BUFFER_SIZE);

    read_size = read(client_fd,message,BUFFER_SIZE);

    printf("Message[0]=%d\n",message[0]);

    printf("Message[1]=%s\n",&message[1]);

    printf("Message read: %s\n",message);

    fwrite(message,sizeof(unsigned char),read_size,file_stream);

    printf("Hash: %lu\n",djb2_hash(message));
    printf("Read from socket %d\n", read_size);

  }while(read_size > 0);

  printf("File written\n");

  fclose(file_stream);

  file_stream = fopen("test.zip","rb");

  read_size = get_file_size(file_stream);

  printf("Size get_file_size: %d\n",read_size);

  unsigned char file[BUFFER_SIZE];

  if(file_stream != NULL){
    printf("File opened \n");
    read_size = fread(file,sizeof(unsigned char),read_size,file_stream);
    printf("Bytes read from test.zip: %d\n",read_size);
    fflush(stdout);

    hash = djb2_hash(file);
    printf("Hash: %lu \n",hash);
  }else{
    perror("Error opening test.zip\n");
  }

  fclose(file_stream);

  //printer(message,read_size);
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
          handle_client(client_fd);
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
