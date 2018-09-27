#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
//#include "wrapper.h"

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

int main(){
    int fd, b_err,l_err;

    struct sockaddr_in addr, dest_addr;

    fd = socket(AF_INET,SOCK_STREAM,0);

    if(fd<0){
        perror("Error creating socket\n");
    }

    /*
    addr.sin_family = AF_INET;
    addr.sin_port = 5556;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    b_err = bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

    if(b_err <0){
        perror("Error binding socket\n");
    }
    */

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(5555);
    inet_aton("127.0.0.1",&dest_addr.sin_addr);

    char  message[]="Testing I am";

    if(connect(fd,(struct sockaddr *)&dest_addr,sizeof(struct sockaddr_in))<0){
        perror("Error connecting socket\n");
    }else{
        printf("Conectiiing\n");
        write(fd,message,strlen(message));
        printf("In theory sent");
    }


}
