#ifndef __MESSAGES_H
#define __MESSAGES_H

struct server_info{
  int id;
  int fd;
  char ip[30];
  char port[5];
};

unsigned long djb2_hash( unsigned char * str);

int get_file_size(char * filename);

unsigned long get_file_hash(char * filename);

void clear_string(char * string);

void print_servers_2(struct server_info *servers, int n_servers);

void printer_index(char * m, int size);

void printer(char * m, int size);
/*
* Wrapper function from
UNIX Network Programming: Sockets Introduction
By Andrew M. Rudoff, Bill Fenner, W. Richard Steven
*/
ssize_t writen(int fd, const void *vptr, size_t n);

/////////////////////////////////////////////////////////////
// SERVER
/////////////////////////////////////////////////////////////

int handle_q_ok(int reg_fd, char * ip, char * port);

int send_register(int fd,char * port);

void handle_r_ok(int reg_fd,int * n_servers,int * my_id);

int send_query(int reg_fd, int id);

int send_register(int reg_fd, char * port);

int handle_start(int reg_fd);

/////////////////////////////////////////////////////////////
// REGISTRY
/////////////////////////////////////////////////////////////

void send_start(int n_servers, struct server_info * servers);

void send_q_ok(int server_fd,char * ip, char * port);

void send_q_err(int server_fd);

void handle_query(int server_fd, char * message, int n_servers, struct server_info * servers);

void handle_register(int id,char * message,int size,int n_servers,struct server_info* servers);

/////////////////////////////////////////////////////////////
// CLIENT
/////////////////////////////////////////////////////////////

int send_put(char * filename, int server_fd);



#endif
