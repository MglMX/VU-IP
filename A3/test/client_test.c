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
#include <errno.h>
#include "../messages/messages.h"

#define BUFFER_SIZE 2000

unsigned long djb2_hash( unsigned char * str)
{
  unsigned long hash = 5381;
  int c;
  while (c = *str++)
    hash = ( ( hash << 5) + hash) + c ; /* hash ∗ 33 + c */
  return hash;
}

unsigned long get_file_hash(char * filename){
  printf("\nGet_file_hash\n");

  char * file;

  get_file_string(filename,&file);

  return djb2_hash(file);
}


int get_file_size(char * filename){

  FILE * file = fopen(filename,"rb");

  fseek(file,0L,SEEK_END); //Seeking end of file

  int file_size = ftell(file); //Getting bytes

  fseek(file,0L,SEEK_SET); //Seeking back to begging

  fclose(file);

  return file_size;
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
  printf(" (%d) \n",size);
}

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

int get_file_string(char * filename, char ** file){
  printf("\n Get_file_string\n");

  FILE * file_fd = fopen(filename,"rb");

  if(file_fd == NULL){
    perror("Get_file_string: Not possible to open file for reading\n");
    exit(1);
  }

  unsigned char buffer[BUFFER_SIZE];

  *file = NULL;

  int size=0;
  int pos;
  int read_size;
  do{
    memset(buffer,'\0',BUFFER_SIZE);
    read_size = fread(buffer,sizeof(char),BUFFER_SIZE,file_fd);
    printf("Copied to the buffer %d bytes\n",read_size);
    if(read_size>0){
      pos=size;
      size+=read_size;
      *file = realloc(*file,size);
      memcpy(&((*file)[pos]),buffer,read_size);
    }
  }while(read_size>0);

  fclose(file_fd);

  //printer(file,size);

  printf("Done putting file in string\n");
  return size;
}


int send_put(int server_fd,char * filename, char * files_dir){

  printf("\n Send_put: %s\n",filename);

  char * message;

  int size_filename = strlen(filename);

  message = malloc(size_filename+2); //Size for the code, filename and '\0'

  int pos=0;
  message[pos]=12; //Attaching PUT code

  pos+=1;
  strcat(&message[pos],filename); //Attaching filename

  pos+=strlen(filename);
  message[pos]='\0';

  pos+=1;

  char * file;

  int size_file = get_file_string(filename,&file);

  int total_size = size_file + pos;

  printf("Size file: %d\nFilename size: %d\npos: %d\ntotal_size: %d\n",size_file,size_filename,pos,total_size);

  message = realloc(message,total_size);

  memcpy(&message[pos],file,size_file);

  writen(server_fd,message,total_size);
}

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

int handle_g_ok(int server_fd,char * filename,char * hash_filename, char * hash_file){
  printf("\nhandle_g_ok\n");

  char code;
  read(server_fd,&code,1);

  if(code==25){ //G_OK
    char message[BUFFER_SIZE];
    memset(message,'\0',BUFFER_SIZE);

    FILE * file_stream;

    file_stream = fopen(filename,"wb");

    int read_size;

    int read_name=0;
    int pos=0;

    do{
      if(!read_name){
        read_size = read(server_fd,message,BUFFER_SIZE);
        sprintf(hash_filename,"%s",message);
        printf("Hash_filename: %s(%zu)\n",hash_filename,strlen(hash_filename));

        pos+=strlen(hash_filename)+1;
        sprintf(hash_file,"%s",&message[pos]);
        printf("Hash_file: %s(%zu)\n",hash_file,strlen(hash_file));

        pos+=strlen(hash_file)+1;

        fwrite(&message[pos],sizeof(unsigned char),read_size-pos,file_stream); //Write read bytes minus the bytes of the filename
        read_name=1;
      }else{
        read_size = read(server_fd,message,BUFFER_SIZE);
        fwrite(&message[pos],sizeof(unsigned char),read_size,file_stream);
      }
      printf("Read from socket %d\n", read_size);
    }while(read_size > 0);

    fclose(file_stream);
    return 1;
  }else if(code == 26){ //G_REDIRECT
    return 2;

  }else if(code == 28){
    return 0;
  }else{
    return -1;
  }
}

void send_get(int server_fd, char * filename){
  printf("\nSend_get\n");
  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int pos=0;
  message[pos]=13; //GET Code

  pos+=1;
  strcat(&message[pos],filename);

  pos+=strlen(filename);
  message[pos]='\0';

  pos+=1;

  writen(server_fd,message,pos);

  printf("End of send_get\n");
}

int main(int argc, char * argv[]){
  if(argc < 3){
    printf("Not enough arguments. Usage: client <(put|get|delete)> <filename>");
  }

  int server_fd = init_client_socket("127.0.0.1","8888");

  char filename[50];
  memset(filename,'\0',50);
  strcpy(filename,argv[2]);

  send_get(server_fd,filename);

  shutdown(server_fd,SHUT_WR);

  char hash_filename[100];
  char hash_file[100];
  memset(hash_filename,'\0',100);
  memset(hash_file,'\0',100);

  handle_g_ok(server_fd,filename,hash_filename,hash_file);


  //printf("File hash: %lu\n",get_file_hash(filename));

  //printer(file,size);

  //<<printf("Size of the file: %d\n",size);

  /*
  send_put(server_fd,filename,NULL);

  shutdown(server_fd,SHUT_WR);

  char * file;
  int size = get_file_string(filename,&file);
  printf("File hash: %lu\n",djb2_hash(file));

  */

}
