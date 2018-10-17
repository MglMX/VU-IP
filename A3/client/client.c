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

int main(int argc, char * argv[]){
  if(argc < 3){
    printf("Not enough arguments. Usage: client <(put|get|delete)> <filename>");
  }

  int server_fd = init_client_socket("127.0.0.1","6666");

  FILE * file_fd;

  file_fd = fopen(argv[2],"rb");

  char filename[50];
  memset(filename,'\0',50);

  strcpy(filename,argv[2]);

  printf("Name of the file: %s\n",filename);

  unsigned char file[BUFFER_SIZE];

  int read_size;


  int pos=0;

  file[pos]=12;

  pos+=1;

  strcat(&file[pos],filename);

  pos+=strlen(filename);

  file[pos]='\0';

  pos+=1;


  do{
    read_size = fread(&file[pos],sizeof(char),BUFFER_SIZE,file_fd);
    printf("fread read_size: %d\n",read_size);

    writen(server_fd,file,read_size);

  }while(read_size>0);

  fclose(file_fd);

  close(server_fd);


  unsigned long hash = djb2_hash(file);

  printf("Hash: %lu \n",hash);

  int file_size = get_file_size(file_fd);

  printf("File size: %d \n",file_size);




}
