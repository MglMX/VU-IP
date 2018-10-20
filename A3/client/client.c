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


int main(int argc, char * argv[]){
  if(argc < 3){
    printf("Not enough arguments. Usage: client <(put|get|delete)> <filename>");
  }

  int server_fd = init_client_socket("127.0.0.1","6666");

  char command[7];

  strcpy(command,argv[1]);

  char filename[50];
  memset(filename,'\0',50);
  strcpy(filename,argv[2]);

  printf("Name of the file: %s\n",filename);
  printf("Filename hash: %lu\n",djb2_hash(filename));

  if(strcmp(command,"put") == 0){
    printf("\nPUT\n");
    send_put(server_fd,filename,NULL); //Send the file

    shutdown(server_fd,SHUT_WR); //Make the socket stop reading on the other side

    char hash_filename[50];
    char hash_file[50];

    memset(hash_filename,'\0',50);
    memset(hash_file,'\0',50);

    int res = handle_p_ok(server_fd,hash_filename,hash_file); //Wait for the hash of the filename and file

    if(res == 1){
      printf("Received: Hash_filename: %s Hash_file: %s\n",hash_filename, hash_file);

    }else if(res == 0){
      printf("Something went wrong when putting file\n");
    }else{
      printf("Wrong code received after put.\n");
    }
  }else if(strcmp(command,"get") == 0){
      send_get(server_fd,filename);

      shutdown(server_fd,SHUT_WR); //Make the socket stop reading on the other side

      char hash_filename[50];
      char hash_file[50];

      memset(hash_filename,'\0',50);
      memset(hash_file,'\0',50);

      int res = handle_g_ok(server_fd,filename,hash_filename,hash_file);

      if(res == 1){
        printf("Received: Hash_filename: %s Hash_file: %s\n",hash_filename, hash_file);

      }else if(res == 0){
        printf("File not found\n");

        close(server_fd);
        exit(1);
      }else{
        printf("Wrong code received after put.\n");
      }

  }else if(strcmp(command,"delete") == 0){

  }else{
    perror("Wrong command. Options are: put, get or delete. \n");
  }

  //sleep(5);
  close(server_fd);

  int file_size = get_file_size(filename);

  printf("File size: %d \n",file_size);

  printf("File hash: %lu\n",get_file_hash(filename));

}
