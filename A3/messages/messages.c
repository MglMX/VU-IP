#include <stdio.h>
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

unsigned long get_file_hash(char * filename){
  FILE * file_fd = fopen(filename,"rb");

  unsigned char file[BUFFER_SIZE];

  int read_size;
  do{
    read_size = fread(file,sizeof(char),BUFFER_SIZE,file_fd);

  }while(read_size>0);

  fclose(file_fd);

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


int handle_q_ok(int reg_fd, char * ip, char * port){
  printf("I am in handle_q_ok\n");

  char message[50];
  memset(message,'\0',50);

  int size_read = read(reg_fd,message,50);

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

void handle_put(int client_fd){
  printf("\nhandle_put.\n");

  unsigned char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  FILE *file_stream;

  int read_size;

  unsigned long hash;

  int read_name = 0;

  char filename[50];

  int pos=0;

  do{
    if(!read_name){
      read_size = read(client_fd,message,BUFFER_SIZE);
      sprintf(filename,"%s",message);
      printf("Filename: %s(%zu)\n",filename,strlen(filename));

      file_stream = fopen(filename,"wb");

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

  char hash_filename[50];
  char hash_file[50];

  memset(hash_filename,'\0',50);
  memset(hash_file,'\0',50);

  sprintf(hash_filename, "%lu",djb2_hash(filename));
  sprintf(hash_file, "%lu",get_file_hash(filename));

  printf("Hash filename: %s\n",hash_filename);

  printf("Hash: %s\n",hash_file);

  printf("Length: %d\n",get_file_size(filename));
  fflush(stdout);
  send_g_ok(client_fd,hash_filename,hash_file);


}

void send_g_ok(int client_fd, char * hash_filename, char * hash_file){
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

  printf("Send_g_ok. Going to write\n");
  writen(client_fd,message,pos);
  printf("Send_g_ok. Written\n");
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
    printf("Sending start to server %d with fd: %d\n",i,servers[i].fd);
    writen(servers[i].fd,&reply,1);
  }
}

/**
* Sends Q_OK message with ip and port of the requested server
*/
void send_q_ok(int server_fd,char * ip, char * port){

  char reply[100];

  printf("strlen(reply): %zu\n",strlen(reply));
  memset(reply,'\0',100);

  reply[0]=21; //Q_OK code

  printf("Send_q_ok: IP: %s PORT: %s \n",ip,port);

  int pos=1;
  strcat(&reply[pos],ip); //21<ip>
  pos += strlen(ip);
  reply[pos]='\0'; //21<ip>'\0'
  pos+=1;
  strcat(&reply[pos],port);
  pos+=strlen(port);
  reply[pos]='\0'; //21<ip>\0<port>\0
  pos+=1;

  printer(reply,pos);

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
  printf("Queried server: %d \n",id);
  fflush(stdout);

  print_servers_2(servers,n_servers);
  if(id<0 || id>n_servers){
    printf("Handle_query: Error id < 0 or id > n_servers");
    fflush(stdout);
    send_q_err(server_fd);
  }else{
    printf("Entro en el else\n");
    char ip[20];
    char port[4];

    memset(ip,'\0',20);
    memset(port,'\0',4);

    printf("Servers[%d].ip = %s(%zu)\n",id,servers[id].ip,strlen(servers[id].ip));
    strcpy(ip,servers[id].ip);
    strcpy(port,servers[id].port);

    printf("Copio ip y port\n");
    printf("Queried server: %d IP: %s PORT: %s\n\n",id,ip,port);
    fflush(stdout);
    send_q_ok(server_fd,"127.0.0.1",port);
  }
}


void send_r_ok(int server_fd,int id,int n_servers){
  char reply[3];
  //memset(reply,'\0',strlen(reply));
  reply[0]=20;
  reply[1]=n_servers;
  reply[2]=id;

  printf("reply[0]=%d,reply[1]=%d,reply[2]=%d \n fd: %d\n",reply[0],reply[1],reply[2],server_fd);
  printf("reply: (%zu) ",strlen(reply));
  printer(reply,strlen(reply));
  writen(server_fd,reply,strlen(reply));
  printf("R_OK sent\n");
}
/**
* Receives the listening port of the server connected. Stores it in the servers array. Replies to the server with 20:n_servers:ID
*/
void handle_register(int id,char * message, int size,int n_servers,struct server_info *servers){
  printer(message,size);

  char port[10];
  strcpy(port,&message[1]); //Copying the port from the message skipping the code

  printf("Server %d connected with port: %s(%zu)\n",id,port,strlen(port));

  strcpy(servers[id].port,port); //Updating port in servers array

  printf("holiiiis\n");
  printf("Server_fd: %d\n", servers[id].fd);
  send_r_ok(servers[id].fd,id,n_servers);

}



/////////////////////////////////////////////////////////////
// CLIENT
/////////////////////////////////////////////////////////////

int handle_g_ok(int server_fd, char * hash_filename, char * hash_file){
  printf("In handle_g_ok\n");

  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int size_read;

  do{
    size_read = read(server_fd,message,BUFFER_SIZE);
    printf("Size read: %d\n",size_read);
  }while(size_read>0);

  int pos=0;

  if(message[pos] == 23){
    pos+=1;
    strcpy(hash_filename,&message[pos]);

    pos+=strlen(hash_filename)+1;
    strcpy(hash_file,&message[pos]);

    printf("Received: Hash_filename: %s Hash_file: %s\n",hash_filename, hash_file);

    return 1;
  }else if(message[pos] == 24){
    return 0;
  }else{
    perror("Expecting handle_g_ok but something else found\n");
    return -1;
  }

}

int send_put(int server_fd,char * filename){
  FILE * file_fd;

  file_fd = fopen(filename,"rb");

  unsigned char file[BUFFER_SIZE];

  int read_size;

  int pos=0;

  file[pos]=12; //Attaching PUT code

  pos+=1;

  strcat(&file[pos],filename); //Attaching filename

  pos+=strlen(filename);

  file[pos]='\0';

  pos+=1;

  do{
    read_size = fread(&file[pos],sizeof(char),BUFFER_SIZE,file_fd);
    printf("fread read_size: %d\n",read_size);

    writen(server_fd,file,read_size);

  }while(read_size>0);

  unsigned long hash = djb2_hash(&file[pos]);

  printf("Hash: %lu \n",hash);

  fclose(file_fd);
}
