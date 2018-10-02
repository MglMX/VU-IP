#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include "keyvalue.h"

int main(){
  init_array(10);

  put("animal","dog");
  put("color","blue");


  if (fork()==0) {
    printf("CHILD\n");
    print_all();
    put("animal","cat");
    put("color","red");
    put("shape","circle");
    printf("-----------\n\n");
    /*
    printf("The key is: %s\n", shared_array[0].key);
    printf("The value is: %s\n", shared_array[0].value);
    printf("The key is: %s\n", shared_array[1].key);
    printf("The value is: %s\n", shared_array[1].value);
    strcpy(shared_array[0].value,"dog");
    strcpy(shared_array[1].value,"green");
    */
    dettach_mem();
  }
  else if (fork()==0) {
    printf("CHILD2\n");
    print_all();
    put("animal","cat");
    put("color","green");
    put("pim","pom");
    printf("-----------\n\n");
    /*
    printf("The key is: %s\n", shared_array[0].key);
    printf("The value is: %s\n", shared_array[0].value);
    printf("The key is: %s\n", shared_array[1].key);
    printf("The value is: %s\n", shared_array[1].value);
    strcpy(shared_array[0].value,"dog");
    strcpy(shared_array[1].value,"green");
    */
    dettach_mem();
  }else{
    sleep(3);
    printf("PARENT\n");
    print_all();
    printf("-----------\n\n");
    /*
    printf("The key is: %s\n", shared_array[0].key);
    printf("The value is: %s\n", shared_array[0].value);
    printf("The key is: %s\n", shared_array[1].key);
    printf("The value is: %s\n", shared_array[1].value);;
    */
    dettach_mem();
    ctl_mem();
  }
}
