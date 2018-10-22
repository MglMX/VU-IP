#ifndef __MESSAGES_H
#define __MESSAGES_H

struct server_info{
  int id;
  int fd;
  char ip[30];
  char port[5];
};

unsigned long djb2_hash( unsigned char * str);

void get_path(char * filename, char * files_dir,char * path);

int delete_file(char * filename, char * files_dir);

int init_client_socket(char * address, char * port);

int get_file_size(char * filename);

int get_file_string(char * filename, char ** file);

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

void send_d_ok(int client_fd);

void send_d_err(int client_fd);

void handle_delete(int client_fd, int reg_fd, int n_servers, int my_id, char * files_dir);

int handle_q_ok(int reg_fd, char * ip, char * port);

int send_register(int fd,char * port);

void handle_r_ok(int reg_fd,int * n_servers,int * my_id);

int send_query(int reg_fd, int id);

int send_register(int reg_fd, char * port);

int handle_start(int reg_fd);

void handle_put(int client_fd,int reg_fd, int n_servers, int my_id, char * files_dir);

void send_p_ok(int client_fd, char * hash_filename, char * hash_file);

void handle_get(int client_fd, int reg_fd, int n_servers, int my_id, char * files_dir);

/////////////////////////////////////////////////////////////
// REGISTRY
/////////////////////////////////////////////////////////////

void send_start(int n_servers, struct server_info * servers);

void send_q_ok(int server_fd,char * ip, char * port);

void send_q_err(int server_fd);

void handle_query(int server_fd, char * message, int n_servers, struct server_info * servers);

void handle_register(int id,char * message,int size,int n_servers,struct server_info* servers);

void send_g_ok(int client_fd, char * filename, char * files_dir);

void send_g_redirect(int client_fd, char *ip,char * port);

/////////////////////////////////////////////////////////////
// CLIENT
/////////////////////////////////////////////////////////////
int handle_g_redirect(int server_fd);

int handle_g_ok(int server_fd,char * filename,char * hash_filename, char * hash_file);

void send_get(int server_fd, char * filename);

int handle_p_ok(int server_fd, char * hash_filename, char * hash_file);

int send_put(int server_fd,char * filename, char * files_dir);

void send_delete(int server_fd, char * filename);

int handle_d_ok(int server_fd);

#endif
