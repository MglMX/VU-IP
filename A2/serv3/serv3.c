#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "wrapper.h"
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "keyvalue.h"

#define BUFFER_SIZE 1000

int sem_accept;
struct sembuf upAcc = {0,1,0};
struct sembuf downAcc = {0,-1,0};

void printer(char * m, int size){
  int i;

  for(i=0;i<size;i++){
    if(m[i]=='\0')
      printf("\\0");
    else
      printf("%c",m[i]);
  }
  printf("\n");
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

int attach_get_reply(char * reply,char * result,int s_pos){
  if(result==NULL){
    reply[s_pos]='n';
    s_pos+=1;
  }else{
    reply[s_pos]='f';
    s_pos+=1;
    strcat(&reply[s_pos],result);
    s_pos+=strlen(result);
    reply[s_pos]='\0';
    s_pos+=1;
  }
  return s_pos;
}

void handle_message(int client_fd,char * message){

  char  command[100];
  char reply[1000];
  memset(reply,'\0',1000);

  int stop=0;
  int s_pos=0;
  int reply_pos=0;

  char key[100];
  char value[100];
  char result[100];

  while(!stop){
    sprintf(command,"%s",&message[s_pos]);
    if(strlen(command)==0){
      stop=1;
    }else{
      if(command[0]=='p'){
        strcpy(key,&command[1]);
        s_pos+=strlen(key)+2;//p<key>\0_

        sprintf(value,"%s",&message[s_pos]);
        s_pos+=strlen(value)+1;

        put(key,value);
      }else if(command[0]=='g'){
        strcpy(key,&command[1]);
        s_pos+=strlen(key)+2; //g<key>\0_

        reply_pos=attach_get_reply(reply,get(key),reply_pos);
      }else{
        perror("Bad format command");
        stop=1;
      }
    }
  }

  
  //printer(reply,reply_pos);
  writen(client_fd,reply,reply_pos);

}

void sig_chld() {
  while (waitpid(0, NULL, WNOHANG) > 0) {}
  signal(SIGCHLD,sig_chld);
}

void treat_request(int client_fd){
  char message[BUFFER_SIZE];
  memset(message,'\0',BUFFER_SIZE);

  int read_size;

  read_size = read(client_fd,message,BUFFER_SIZE);
  //printer(message,read_size);
  handle_message(client_fd,message);
  //print_all();

  //dettach_mem();
}

void recv_requests(int fd,struct sockaddr * client_addr,socklen_t *addrlen, int child) { /* An iterative server */
  int newfd;
  while (1) {
    //printf("Child %d before semaphore to accept\n",child);
    semop(sem_accept,&downAcc,1);
    newfd = accept(fd,client_addr,addrlen);
    //printf("Child %d Request accepted \n",child);
    semop(sem_accept,&upAcc,1);
    treat_request(newfd);
    close(newfd);
    //printf("Child %d DONE. newfd closed\n",child);
    //exit(1);
  }
}
int main(int argc, char *argv[]){
    if(argc < 3){
      perror("Missing port number and/or n_process\n");
      exit(1);
    }
    /*
    put("color","pink");
    printf("--Color: %s\n",get("color"));
    put("city","Granada");
    printf("--City: %s\n",get("city"));
    printf("--Color: %s\n",get("color"));
    */

    init_array(100);

    int n_proc = atoi(argv[2]);
    int fd;
    fd = init_socket(atoi(argv[1])); //Change to argv[1]

    int client_fd;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in client_addr;

    sem_accept = semget(IPC_PRIVATE,1,0600);
    semop(sem_accept,&upAcc,1);

    int pid;
    signal(SIGCHLD, sig_chld);

    int i;
    for(i=0;i<n_proc;i++){
      if(fork()==0)
        recv_requests(fd,(struct sockaddr *)&client_addr,&addrlen,i);
    }


    int status;
    while (wait(&status) > 0){ };

    dettach_mem();
    ctl_mem();
}
