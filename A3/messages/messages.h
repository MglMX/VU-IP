#ifndef __MESSAGES_H
#define __MESSAGES_H

struct server_info{
  int id;
  int fd;
  char * ip;
  char * port;
};


void printer(char * m, int size);
/*
* Wrapper function from
UNIX Network Programming: Sockets Introduction
By Andrew M. Rudoff, Bill Fenner, W. Richard Steven
*/
ssize_t writen(int fd, const void *vptr, size_t n);

//Sends the message 10:PORT to the registry
int send_register(int fd,char * port);

void handle_r_ok(int reg_fd,int * n_servers,int * my_id);

//////////////////////////////////////////////////7

void handle_register(int id,char * message,int size,int n_servers,struct server_info* servers);

#endif
