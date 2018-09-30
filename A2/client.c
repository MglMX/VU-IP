#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "wrapper.h"
#include <stdlib.h>

#define BUFFER_SIZE 1000
/*
struct sockaddr_in {
    sa_family_t sin_family; // set to AF_INET
    in_port_t sin_port; // Port number
    struct in_addr sin_addr; // Contains the IP address
};

struct in_addr {
    in_addr_t s_addr;
};
*/

void printer(char * m, int size){
  int i;

  for(i=0;i<size;i++){
    if(m[i]=='\0')
      printf("\\0");
    else
      printf("%c",m[i]);
  }
}

int attach_get(char * message, char * key, int s_pos, int first){
  int i_pos = s_pos;

  printf("Key_ %s\n",key);

  if(first){ //Message not initalized
    strcpy(message,"g"); //Equivalent to 103
  }else{
    strcat(&message[s_pos],"g");
  }

  s_pos += 1;//Next writting position g_

  strcat(&message[s_pos],key); //Concat the key to the message

  s_pos += strlen(key); //Next writting position g<key>_

  message[s_pos]='\0'; //Add \0 to finish command

  printf("Message to be sent: %s(%zu)\n",&message[i_pos],strlen(&message[i_pos]));

  s_pos += 1;//Position where next command should start g<key>\0_

  printf("s_pos: %d\n",s_pos);

  return s_pos;
}

int attach_put(char * message, char * key, char * value, int s_pos, int first){
  int i_pos = s_pos;

  printf("Key_ %s\nValue_ %s\n",key,value);

  if(first){ //Message not initalized
    strcpy(message,"p"); //Equivalent to 103
  }else{
    strcat(&message[s_pos],"p");
  }

  s_pos += 1;
  strcat(&message[s_pos],key); //Concat the key to the message

  s_pos += strlen(key);

  message[s_pos]='\0'; //Add \0 to finish command

  s_pos+=1; //p<key>\0_

  strcat(&message[s_pos],value);
  s_pos+=strlen(value); //p<key>\0<value>_
  message[s_pos]='\0';

  s_pos+= 1;//p<key>\0<value>\0_

  printf("Message to be sent: %s(%zu)\n",&message[i_pos],strlen(&message[i_pos]));
  printf("s_pos: %d\n",s_pos);

  return s_pos;
}

int init_socket(char * address, char * port){
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



int main(int argc, char *argv[]){

    if(argc < 5){
      printf("Not enough arguments");
      exit(1);
    }

    int arg_check = 3; //Argument that needs to be checked

    int sfd = init_socket(argv[1],argv[2]);

    char message[BUFFER_SIZE];

    memset(message,'\0',BUFFER_SIZE);
    int s_pos = 0; //Position where command should be append

    while(argv[arg_check] != NULL){

      printf("I am in the while\n");

      if(strcmp(argv[arg_check],"get")==0){
          char * key = argv[arg_check+1]; //After get we get the key

          int first = 0;

          if(arg_check==3)
            first=1; //First argument

          s_pos = attach_get(message,key,s_pos,first);

          arg_check+=2; // Next argument to check from argv

      }else if(strcmp(argv[arg_check],"put")==0){

          char * key = argv[arg_check+1]; //After put we get the key
          char * value = argv[arg_check+2]; //After key we get the value

          int first = 0;

          if(arg_check==3)
            first=1; //First argument

          s_pos = attach_put(message,key,value,s_pos,first);

          arg_check+=3; // Next argument to check from argv

      }else{
        //Wrong syntax
      }
    }
    printer(message,s_pos);
    printf("\n");

    writen(sfd,message,BUFFER_SIZE);

    int read_size;
    char reply[BUFFER_SIZE];
    memset(reply,'\0',BUFFER_SIZE);


    printf("Going to read\n");
    read_size = readn(sfd,reply,BUFFER_SIZE);

    printf("Message received: %s\n",reply);
    fflush(stdout);
    printer(reply,read_size);

}
