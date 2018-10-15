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
  memset(message,'\0',strlen(message));
  int size_read = read(reg_fd,message,50);
  printer(message,size_read);
  
  if(message[0]==21){
    printf("Handle_q_ok: Code is 21\n");
    int pos=1;
    strcpy(ip,&message[pos]);
    printf("Handle_q_ok: IP: %s \n",ip);
    fflush(stdout);
    pos+=strlen(ip)+1;
    printf("Handle_q_ok: 1.PORT: %s \n",&message[11]);
    strcpy(port,&message[11]);
    printf("Handle_q_ok: 2.PORT: %s \n",port);
    fflush(stdout);
    printf("Handle_q_ok: IP: %s PORT: %s \n",ip,port);
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
