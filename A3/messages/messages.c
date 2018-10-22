#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "messages.h"
#include <errno.h>
#include <stdlib.h>

#define BUFFER_SIZE 2000

unsigned long djb2_hash( unsigned char * str)
{
  unsigned long hash = 5381;
  int c;
  while (c = *str++)
    hash = ( ( hash << 5) + hash) + c ; /* hash ∗ 33 + c */
  return hash;
}

void get_path(char * filename, char * files_dir,char * path){
  int size_files_dir = strlen(files_dir);
  int size_filename = strlen(filename);

  strcpy(path,files_dir); //Copying dir
  strcat(&path[size_files_dir],filename); //Attaching Filename

  path[size_files_dir+size_filename]='\0';
}

int delete_file(char * filename, char * files_dir){
  if(files_dir == NULL){
    return remove(filename);
  }else{
    char path[100];
    memset(path,'\0',100);

    strcpy(path,files_dir);
    strcat(&path[strlen(files_dir)],filename);

    return remove(path);
  }
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

unsigned long get_file_hash(char * filename){
  //printf("\nGet_file_hash: %s\n",filename);
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

int get_file_string(char * filename, char ** file){
  //printf("\n Get_file_string\n");

  //fflush(stdout);

  FILE * file_fd = fopen(filename,"rb");

  if(file_fd == NULL){
    perror("Get_file_string: Not possible to open file for reading\n");
    return -1;
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

void printer_index(char * m, int size){
  int i;

  for(i=0;i<size;i++){
    if(i == 0){
      printf("0. %d\n",m[i]);
    }else{
      if(m[i]=='\0')
        printf("%d \\0\n",i);
      else
        printf("%d %c\n",i,m[i]);
    }

  }
  printf(" (%d) \n",size);
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

/////////////////////////////////////////////////////////////
// SERVER
/////////////////////////////////////////////////////////////

void send_d_ok(int client_fd){
  char code = 29; //D_OK Code
  writen(client_fd,&code,1);
}

void send_d_err(int client_fd){
  char code = 30; //D_OK Code
  writen(client_fd,&code,1);
}

void handle_delete(int client_fd, int reg_fd, int n_servers, int my_id, char * files_dir){
  printf("\nhandle_delete. Server %d\n",my_id);

  unsigned char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int read_size;
  do{
    read_size = read(client_fd,message,BUFFER_SIZE);
  }while(read_size > 0);

  char filename[50];
  memset(filename,'\0',50);

  sprintf(filename, "%s",message);

  unsigned long hash = djb2_hash(filename);
  printf("Hash filename: %lu\n",djb2_hash(filename));

  int target_server = hash%n_servers;
  printf("File should be in server: %d\n",target_server);

  if(target_server == my_id){
    printf("I am the server who has the file to be deleted.(%d)\n",my_id);

    int res = delete_file(filename,files_dir);

    if(res == 0){
      send_d_ok(client_fd);
    }else{
      send_d_err(client_fd);
    }

  }else{
    char ip[30];
    char port[5];

    memset(ip,'\0',30);
    memset(port,'\0',5);

    send_query(reg_fd,target_server);

    handle_q_ok(reg_fd,ip,port);

    printf("Target server ip: %s\nTarget server port: %s\n",ip,port);

    int target_server_fd = init_client_socket(ip,port);

    printf("Going to send_delete: %s to target_server. ",filename);
    send_delete(target_server_fd,filename);

    shutdown(target_server_fd,SHUT_WR);

    int res = handle_d_ok(target_server_fd);

    if(res == 1){
      send_d_ok(client_fd);
    }else if (res == 0){
      send_d_err(client_fd);
    }else{
      perror("Received somehitng unexpected\n");
    }

    //TODO: Handle_d_ok

    close(target_server_fd);
  }

}

int handle_q_ok(int reg_fd, char * ip, char * port){
  printf("I am in handle_q_ok\n");

  char message[50];
  memset(message,'\0',50);

  int size_read = read(reg_fd,message,16);

  printer(message,size_read);

  if(message[0]==21){
    printf("Handle_q_ok: Code is 21\n");

    int pos=1;
    sprintf(ip,"%s",&message[pos]);
    pos+=strlen(ip)+1;
    sprintf(port,"%s",&message[pos]);

    printf("Handle_q_ok: IP: %s PORT: %s\n",ip,port);

    return 1;
  }else if(message[0]==22){
    return 0;
  }else{
    return -1;
  }
}

void handle_r_ok(int reg_fd,int * n_servers,int * my_id){
  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);
  int read_size = read(reg_fd,message,3);
  printf("handle_r_ok: Message received:(%d) ",read_size);

  int i;
  for(i=0; i<read_size;i++){
    printf("%d:",message[i]);
  }
  printf("\n");

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
  message[strlen(port)+1]='\0';

  return writen(reg_fd,message,strlen(message));

}

int handle_start(int reg_fd){
  printf("I am in handle_start\n");
  char message[BUFFER_SIZE];
  memset(message,'\0',strlen(message));
  int size_read = read(reg_fd,message,1);
  printf("I am in handle_start. Message read \n");
  printer(message,size_read);
  if(message[0]==40){
    return 1;
  }else{
    return 0; //Error
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
  strcpy(path,files_dir); //Copying dir to path

  int pos=0;

  do{
    if(!read_name){
      read_size = read(client_fd,message,BUFFER_SIZE);
      sprintf(filename,"%s",message);
      printf("Filename: [%s](%zu)\n",filename,strlen(filename));
      strcat(&path[strlen(files_dir)],filename); //Attaching filename to path
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

  printf("Stream closed\n");
  char hash_filename[100];
  char hash_file[100];

  memset(hash_filename,'\0',100);
  memset(hash_file,'\0',100);

  printf("Going to hash filename: %s\n",filename);
  sprintf(hash_filename, "%lu",djb2_hash(filename));
  printf("Hash filename: %s\n",hash_filename);


  printf("Going to hash file in path: %s\n",path);
  printf("Hash on the fly: %lu\n",get_file_hash(path));
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
    char ip[30];
    char port[5];

    memset(ip,'\0',30);
    memset(port,'\0',5);

    send_query(reg_fd,target_server);

    handle_q_ok(reg_fd,ip,port);

    printf("Target server ip: %s\nTarget server port: %s\n",ip,port);

    int target_server_fd = init_client_socket(ip,port);

    printf("Going to send_put: %s to target_server. ",filename);
    send_put(target_server_fd,filename,files_dir);

    shutdown(target_server_fd,SHUT_WR);

    memset(hash_filename,'\0',50);
    memset(hash_file,'\0',50);

    int res = handle_p_ok(target_server_fd,hash_filename,hash_file); //Wait for the hash of the filename and file

    close(target_server_fd);

    if(res == 1){
      //printf("Received from target server: Hash_filename: %s Hash_file: %s\n",hash_filename, hash_file);
      send_p_ok(client_fd,hash_filename,hash_file);
      delete_file(filename,files_dir);

    }else if(res == 0){
      //printf("File not found in target server\n");
      char code = 24;
      writen(client_fd,&code,1);
    }else{
      printf("Wrong code received after put to target server.\n");
    }

  }
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

  writen(client_fd,message,pos);

}

void send_g_redirect(int client_fd, char *ip,char * port){
  char reply[100];
  memset(reply,'\0',100);

  reply[0]=26; //Q_OK code

  int pos=1;
  strcat(&reply[pos],ip); //21<ip>
  pos += strlen(ip);
  reply[pos]='\0'; //21<ip>'\0'
  pos+=1;
  strcat(&reply[pos],port);
  pos+=strlen(port);
  reply[pos]='\0'; //21<ip>\0<port>\0
  pos+=1;

  writen(client_fd,reply,pos);
}

void handle_get(int client_fd, int reg_fd, int n_servers, int my_id, char * files_dir){

  unsigned char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int read_size;
  do{
    read_size = read(client_fd,message,BUFFER_SIZE);
  }while(read_size > 0);

  char filename[50];
  memset(filename,'\0',50);

  sprintf(filename, "%s",message);

  unsigned long hash = djb2_hash(filename);

  int target_server = hash%n_servers;
  //printf("File should be in server: %d\n",target_server);

  if(target_server == my_id){
    printf("I am the server who has the file.(%d)\n",my_id);
    send_g_ok(client_fd,filename,files_dir);
  }else{
    char ip[30];
    char port[5];

    memset(ip,'\0',30);
    memset(port,'\0',5);

    send_query(reg_fd,target_server);

    handle_q_ok(reg_fd,ip,port);

    //printf("Target server ip: %s\nTarget server port: %s\n",ip,port);

    send_g_redirect(client_fd,ip,port);
    //TODO: Remote get
  }
}

void send_g_err(int client_fd){
  char code = 28;
  writen(client_fd,&code,1);
}

void send_g_ok(int client_fd, char * filename, char * files_dir){
  printf("\nSend_g_ok\n");

  char path[100];
  memset(path,'\0',100);
  get_path(filename,files_dir,path);

  char * check;

  if(get_file_string(path,&check) == -1){//File does not exist
    send_g_err(client_fd);
  }else{
    char * message;

    char hash_filename[50];
    char hash_file[50];
    memset(hash_filename,'\0',50);
    memset(hash_file,'\0',50);

    sprintf(hash_filename,"%lu",djb2_hash(filename));
    sprintf(hash_file,"%lu",get_file_hash(path));

    //printf("Going to send hash_filename: %s hash_file: %s\n",hash_filename,hash_file);

    int size_hash_filename = strlen(hash_filename);
    int size_hash_file = strlen(hash_file);
    int size_hashes = size_hash_filename+size_hash_file;

    char * file;

    int size_file = get_file_string(path,&file);
    int size_total = size_hashes+size_file+3;//Size for code + hashes + 2*'\0' + file

    //printf("size_hash_filename: %d\nsize_hash_file: %d\nsize_hashes: %d\nsize_file: %d\nsize_total: %d\n",size_hash_filename, size_hash_file,size_hashes,size_file,size_total);

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

    writen(client_fd,message,pos);
  }
}

/////////////////////////////////////////////////////////////
// REGISTRY
/////////////////////////////////////////////////////////////

/**
* Sends start code to all the servers
*/
void send_start(int n_servers, struct server_info * servers){
  char reply;
  reply=40; // START code
  int i;
  for(i=0;i<n_servers;i++){
    //printf("Sending start to server %d with fd: %d\n",i,servers[i].fd);
    writen(servers[i].fd,&reply,1);
  }
}

/**
* Sends Q_OK message with ip and port of the requested server
*/
void send_q_ok(int server_fd,char * ip, char * port){

  //printf("Send_q_ok\n");
  char reply[100];
  memset(reply,'\0',100);

  reply[0]=21; //Q_OK code

  //printf("Send_q_ok: IP: %s PORT: %s \n",ip,port);

  int pos=1;
  strcat(&reply[pos],ip); //21<ip>
  pos += strlen(ip);
  reply[pos]='\0'; //21<ip>'\0'
  pos+=1;
  strcat(&reply[pos],port);
  pos+=strlen(port);
  reply[pos]='\0'; //21<ip>\0<port>\0
  pos+=1;

  //printer(reply,pos);

  writen(server_fd,reply,pos);
}

/**
* Sends Q_ERR message
*/
void send_q_err(int server_fd){
  char reply[1];

  reply[0]=22; //Q_ERR code

  writen(server_fd,reply,1);
}
/**
* Checks the ip and port of server with id id in the servers array and returns it to the server that requested it
* Sends R_OK
**/
void handle_query(int server_fd, char * message, int n_servers, struct server_info * servers){

  int id = message[1];

  if(id<0 || id>n_servers){ //ID is out of range
    perror("Handle_query: Error id < 0 or id > n_servers");
    send_q_err(server_fd);
  }else{
    char ip[20];
    char port[4];

    memset(ip,'\0',20);
    memset(port,'\0',4);

    //printf("Servers[%d].ip = %s(%zu)\n",id,servers[id].ip,strlen(servers[id].ip));
    strcpy(ip,servers[id].ip);
    strcpy(port,servers[id].port);


    send_q_ok(server_fd,"127.0.0.1",port);
  }
}


/**
* Sends the number of server and its ID to the server that just registered
*/
void send_r_ok(int server_fd,int id,int n_servers){
  char reply[3];
  //memset(reply,'\0',strlen(reply));
  reply[0]=20;
  reply[1]=n_servers;
  reply[2]=id;

  //printer(reply,strlen(reply));
  writen(server_fd,reply,3);
}
/**
* Receives the listening port of the server connected. Stores it in the servers array. Replies to the server with 20:n_servers:ID
*/
void handle_register(int id,char * message, int size,int n_servers,struct server_info *servers){
  printer(message,size);

  char port[10];
  strcpy(port,&message[1]); //Copying the port from the message skipping the code

  //printf("Server %d connected with port: %s(%zu)\n",id,port,strlen(port));

  strcpy(servers[id].port,port); //Updating port in servers array

  send_r_ok(servers[id].fd,id,n_servers);

}



/////////////////////////////////////////////////////////////
// CLIENT
/////////////////////////////////////////////////////////////

/**
* Sends get filename request to the server
*/
void send_get(int server_fd, char * filename){
  //printf("\nSend_get\n");
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

  //printf("End of send_get\n");
}

/**
* It returns the file descriptor of the target server
*/
int handle_g_redirect(int server_fd){
  //printf("\nhandle_g_redirect\n");
  char message[50];
  memset(message,'\0',50);

  int size_read = read(server_fd,message,50);

  char ip[30];
  char port[5];

  memset(ip,'\0',30);
  memset(port,'\0',5);


  sprintf(ip,"%s",&message[0]);
  sprintf(port,"%s",&message[strlen(ip)+1]);


  int target_server_fd = init_client_socket(ip,port);

  return target_server_fd;
}

int handle_g_ok(int server_fd,char * filename,char * hash_filename, char * hash_file){

  char code;
  read(server_fd,&code,1);

  if(code==25){ //G_OK
    unsigned char message[BUFFER_SIZE];
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
        //printf("Hash_filename: %s(%zu)\n",hash_filename,strlen(hash_filename));

        pos+=strlen(hash_filename)+1;
        sprintf(hash_file,"%s",&message[pos]);
        //printf("Hash_file: %s(%zu)\n",hash_file,strlen(hash_file));

        pos+=strlen(hash_file)+1;

        fwrite(&message[pos],sizeof(unsigned char),read_size-pos,file_stream); //Write read bytes minus the bytes of the filename
        read_name=1;
      }else{
        read_size = read(server_fd,message,BUFFER_SIZE);
        fwrite(message,sizeof(unsigned char),read_size,file_stream);
      }
      //printf("Read from socket %d\n", read_size);
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

int handle_p_ok(int server_fd, char * hash_filename, char * hash_file){

  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int size_read;

  do{
    size_read = read(server_fd,message,BUFFER_SIZE);
    //printf("Size read: %d\n",size_read);
  }while(size_read>0);

  int pos=0;

  if(message[pos] == 23){
    pos+=1;
    strcpy(hash_filename,&message[pos]);

    pos+=strlen(hash_filename)+1;
    strcpy(hash_file,&message[pos]);

    //printf("Received: Hash_filename: %s Hash_file: %s\n",hash_filename, hash_file);

    return 1;
  }else if(message[pos] == 24){
    return 0;
  }else{
    perror("Expecting handle_p_ok but something else found\n");
    return -1;
  }
}

void send_delete(int server_fd, char * filename){

  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int pos=0;
  message[pos]=14; //DELETECode

  pos+=1;
  strcat(&message[pos],filename);

  pos+=strlen(filename);
  message[pos]='\0';

  pos+=1;

  writen(server_fd,message,pos);
}


int handle_d_ok(int server_fd){
  char code;
  read(server_fd,&code,1);

  if(code == 29){ //D_OK
    return 1;
  }else if(code == 30){ //D_ERROR
    return 0;
  }else{
    return -1;
  }
}
int send_put(int server_fd,char * filename, char * files_dir){

  char * message;

  int size_filename = strlen(filename);

  message = malloc(size_filename+2); //Size for the code, filename and '\0'
  memset(message,'\0',size_filename+2);

  int pos=0;
  message[pos]=12; //Attaching PUT code

  pos+=1;
  strcat(&message[pos],filename); //Attaching filename

  pos+=strlen(filename);
  message[pos]='\0';

  pos+=1;

  char * file;

  int size_file;

  if(files_dir == NULL){
    size_file = get_file_string(filename,&file);
  }else{
    char path[50];
    memset(path,'\0',50);
    get_path(filename,files_dir,path);

    size_file = get_file_string(path,&file);
  }

  int total_size = size_file + pos;

  //printf("Size file: %d\nFilename size: %d\npos: %d\ntotal_size: %d\n",size_file,size_filename,pos,total_size);

  message = realloc(message,total_size);
  memset(&message[pos],0,size_file);

  memcpy(&message[pos],file,size_file);

  writen(server_fd,message,total_size);
}
