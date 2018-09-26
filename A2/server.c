#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<string.h>

int main(){
    int fd, b_err,l_err;

    struct sockaddr_in addr, client_addr;

    fd = socket(AF_INET,SOCK_STREAM,0); //Create a socket

    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) //Enabling reuseaddr
      perror("setsockopt(SO_REUSEADDR) failed");

    if(fd<0){
        perror("Error creating socket\n");
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    b_err = bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)); //Binf the socket

    if(b_err <0){
        perror("Error binding socket\n");
    }

    l_err = listen(fd,5); //Make the server listen

    int client_fd;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    while(1){
        client_fd = accept(fd,(struct sockaddr *)&client_addr,&addrlen); //Accept connections to this socket
        if(client_fd<0){
            perror("Error after accept\n");
        }else{
            printf("Aqui está el tio conectao\n");
        }
    }


}
