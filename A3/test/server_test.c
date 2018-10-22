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
#include <errno.h>
#include "../messages/messages.h"

#define BUFFER_SIZE 2000

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

unsigned long djb2_hash( unsigned char * str)
{
  unsigned long hash = 5381;
  int c;
  while (c = *str++)
    hash = ( ( hash << 5) + hash) + c ; /* hash ∗ 33 + c */
  return hash;
}

int get_file_string(char * filename, char ** file){
  //printf("\n Get_file_string\n");

  //fflush(stdout);

  FILE * file_fd = fopen(filename,"r");

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
    //printf("Copied to the buffer %d bytes\n",read_size);
    if(read_size>0){
      pos=size;
      size+=read_size;
      *file = realloc(*file,size);
      memcpy(&((*file)[pos]),buffer,read_size);
    }
  }while(read_size>0);

  fclose(file_fd);

  //printer(file,size);

  //printf("Done putting file in string\n");
  return size;
}

unsigned long get_file_hash(char * filename){
  printf("\n Get_file_hash\n");

  FILE * file_fd = fopen(filename,"rb");

  if(file_fd == NULL){
    perror("Get_file_hash: Not possible to open file for reading\n");
  }

  unsigned char buffer[BUFFER_SIZE];

  unsigned char * file;

  int size=0;
  int pos;
  int read_size;
  do{
    memset(buffer,'\0',BUFFER_SIZE);
    read_size = fread(buffer,sizeof(char),BUFFER_SIZE,file_fd);
    if(read_size>0){
      pos=size;
      size+=read_size;
      file = realloc(file,size);
      memcpy(&file[pos],buffer,read_size);
    }
  }while(read_size>0);

  fclose(file_fd);

  //printer(file,size);
  return djb2_hash(file);

}

void get_path(char * filename, char * files_dir,char * path){
  int size_files_dir = strlen(files_dir);
  int size_filename = strlen(filename);

  strcpy(path,files_dir); //Copying dir
  strcat(&path[size_files_dir],filename); //Attaching Filename

  path[size_files_dir+size_filename]='\0';
}

int get_file_size(char * filename){

  FILE * file = fopen(filename,"rb");

  fseek(file,0L,SEEK_END); //Seeking end of file

  int file_size = ftell(file); //Getting bytes

  fseek(file,0L,SEEK_SET); //Seeking back to begging

  fclose(file);

  return file_size;
}

void clear_string(char * string){
  memset(string,'\0',strlen(string));
}

void print_servers_2(struct server_info *servers, int n_servers){
  int i;
  for(i=0;i<n_servers;i++){
    printf("servers[%d]:{\n id: %d\n fd: %d\n ip: %s\n port:%s\n}\n",i,servers[i].id,servers[i].fd,servers[i].ip,servers[i].port);
  }
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

void send_p_ok(int client_fd, char * hash_filename, char * hash_file){
  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int pos=0;
  message[pos]=23; //PUT Code

  pos+=1;
  strcat(&message[pos],hash_filename);

  pos+=strlen(hash_filename);
  message[pos]='\0';

  pos+=1;
  strcat(&message[pos],hash_file);

  pos+=strlen(hash_file);
  message[pos]='\0';

  //pos+=1;

  printf("Send_p_ok. Going to write\n");
  writen(client_fd,message,pos);
  printf("Send_p_ok. Written\n");
}

void send_g_ok(int client_fd, char * filename, char * files_dir){
  printf("\nSend_g_ok\n");

  char * message;

  char path[100];
  get_path(filename,files_dir,path);

  char hash_filename[50];
  char hash_file[50];
  memset(hash_filename,'\0',50);
  memset(hash_file,'\0',50);

  sprintf(hash_filename,"%lu",djb2_hash(filename));
  sprintf(hash_file,"%lu",get_file_hash(path));

  printf("Going to send hash_filename: %s hash_file: %s\n",hash_filename,hash_file);

  int size_hash_filename = strlen(hash_filename);
  int size_hash_file = strlen(hash_file);
  int size_hashes = size_hash_filename+size_hash_file;

  char * file;

  int size_file = get_file_string(path,&file);
  int size_total = size_hashes+size_file+3;//Size for code + hashes + 2*'\0' + file

  printf("File: %s\n",file);
  printf("size_hash_filename: %d\nsize_hash_file: %d\nsize_hashes: %d\nsize_file: %d\nsize_total: %d\n",size_hash_filename, size_hash_file,size_hashes,size_file,size_total);

  message = malloc(size_total);
  memset(message,'\0',size_hashes+3);
  memset(&message[size_hashes+3],0,size_file);

  int pos = 0;
  message[pos]=25; //G_OK Code

  pos+=1;
  strcat(&message[pos],hash_filename); //Attaching hash_filename

  pos+=size_hash_filename;
  message[pos]='\0';

  pos+=1;
  strcat(&message[pos],hash_file);

  pos+=size_hash_file;
  message[pos]='\0';

  pos+=1;
  strcat(&message[pos],file);

  pos+=size_file;

  printer(message,pos);

  writen(client_fd,message,pos);

  shutdown(client_fd,SHUT_WR);

  printf("Done sending message\n");
}


void handle_get(int client_fd, int reg_fd, int n_servers, int my_id, char * files_dir){
  printf("\nhandle_get.Server %d\n",my_id);

  unsigned char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int read_size;
  do{
    read_size = read(client_fd,message,BUFFER_SIZE);
  }while(read_size > 0);

  char filename[50];
  memset(filename,'\0',50);

  sprintf(filename, "%s",message);

  printf("Filename received: %s\n",filename);

  unsigned long hash = djb2_hash(filename);
  printf("Hash filename: %lu\n",djb2_hash(filename));

  int target_server = hash%n_servers;
  printf("File should be in server: %d\n",target_server);

  if(target_server == my_id){
    printf("I am the server who has the file.(%d)\n",my_id);
    send_g_ok(client_fd,filename,files_dir);
  }
}

void handle_put(int client_fd,int reg_fd, int n_servers, int my_id, char * files_dir){
  printf("\nhandle_put.\n");

  unsigned char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  FILE *file_stream;

  int read_size;

  unsigned long hash;

  int read_name = 0;

  char filename[50];
  memset(filename,'\0',50);

  char path[50];
  memset(path,'\0',50);
  strcpy(path,files_dir);

  int pos=0;

  do{
    if(!read_name){
      read_size = read(client_fd,message,BUFFER_SIZE);
      sprintf(filename,"%s",message);
      printf("Filename: %s(%zu)\n",filename,strlen(filename));
      strcat(&path[strlen(files_dir)],filename);
      printf("Path: %s\n",path);

      file_stream = fopen(path,"wb");
      if(file_stream == NULL){
        perror("Not possible to create file\n");
      }
      pos=strlen(filename)+1;
      fwrite(&message[pos],sizeof(unsigned char),read_size-pos,file_stream); //Write read bytes minus the bytes of the filename
      read_name=1;
    }else{
      read_size = read(client_fd,message,BUFFER_SIZE);
      fwrite(message,sizeof(unsigned char),read_size,file_stream);
    }

    printf("Read from socket %d\n", read_size);

  }while(read_size > 0);

  printf("File written\n");

  fclose(file_stream);

  /*
  printf("Stream closed\n");
  char hash_filename[50];
  char hash_file[50];

  memset(hash_filename,'\0',50);
  memset(hash_file,'\0',50);

  printf("Going to hash filename: %s\n",filename);
  sprintf(hash_filename, "%lu",djb2_hash(filename));
  printf("Hash filename: %s\n",hash_filename);

  printf("Going to hash file in path: %s\n",path);
  printf("HAsh on the fly: %lu\n",get_file_hash(path));
  sprintf(hash_file, "%lu",get_file_hash(path));
  printf("Hash: %s\n",hash_file);




  printf("Length: %d\n",get_file_size(path));

  int target_server = djb2_hash(filename)%n_servers;

  printf("File should be in server %d\n",target_server);

  if(target_server == my_id){
    printf("I am the server who should have this file(%d)\n",my_id);
    send_p_ok(client_fd,hash_filename,hash_file);
  }else{
    printf("\nFile should be in server %d\n",target_server);

  }
  */
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

  int server_fd = init_server_socket(atoi(argv[1]));

  int client_fd;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  struct sockaddr_in client_addr;

  while(1){
      client_fd = accept(server_fd,(struct sockaddr *)&client_addr,&addrlen);
      if(client_fd<0){
        perror("Error after accept\n");
      }else{
        char code;
        read(client_fd,&code,1);
        printf("Received code: %d\n",code);
        handle_get(client_fd,23,1,0,files_dir);
      }
  }
}
