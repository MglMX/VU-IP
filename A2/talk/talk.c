#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

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

void child_read_and_print(int fd){
  char byte;
  read(fd,&byte,1);

  if(byte == '\n'){
    printf("NEWLINEread");
    system ("/bin/stty cooked");
    printf("\r\n");
    fflush(stdout);
    system ("/bin/stty raw");
  }else if(byte == 3){
    close(fd);
    printf("BYEread!");
    fflush(stdout);
    system ("/bin/stty cooked");
    exit(1);
  }else{
    printf("%c",byte);
    fflush(stdout);
  }
}

void parent_write_and_send(int fd){
  char input;

  scanf("%c",&input);

  if(input== '\n'){
    printf("NEWLINEwritten");
    printf("\r\n");
    fflush(stdout);
    system ("/bin/stty raw");

  }else if(input == 3){
    write(fd,&input,1);
    close(fd);
    printf("BYE!");
    fflush(stdout);
    system ("/bin/stty cooked");
    exit(1);
  }

  write(fd,&input,1);
}
void sig_chld() {
  while (waitpid(0, NULL, WNOHANG) > 0) {}
  exit(1);
  signal(SIGCHLD,sig_chld);
}
int main(int argc, char *argv[]){

  if(argc == 2){
    //Server
    printf("I am the server\n");
    int serv_fd = init_server_socket(atoi(argv[1]));

    int client_fd;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in client_addr;

    client_fd = accept(serv_fd,(struct sockaddr *)&client_addr,&addrlen);
    if(client_fd <0){
      perror("Problem accepting client\n");
    }else{
      printf("Client connected \n");
      pid_t pid = fork();
      if(pid==0){
        while(1){
          child_read_and_print(client_fd);
        }
      }else{
        //signal(SIGCHLD,sig_chld);
        system ("/bin/stty raw");
        while (1) {
          parent_write_and_send(client_fd);
        }
        exit(1);
      }
    }
  }else if(argc == 3){
    //Client
    int s_fd = init_client_socket(argv[1],argv[2]);
    printf("I am the client\n");
    pid_t pid = fork();
    if(pid==0){
      while(1){
        child_read_and_print(s_fd);
      }
    }else{
      //signal(SIGCHLD,sig_chld);
      system ("/bin/stty raw");
      while (1) {
        parent_write_and_send(s_fd);
      }
      exit(1);
    }
  }
}
