#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>

struct pair{
  char key[100];
  char value[100];
};

int main(){
  struct pair *shared_array;
  int size = sizeof(struct pair)*2;
  int shmid = shmget(IPC_PRIVATE, size, 0600);
  shared_array = (struct pair *) shmat(shmid, 0, 0);
  strcpy(shared_array[0].key,"animal");
  strcpy(shared_array[0].value,"hedgehog");
  strcpy(shared_array[1].key,"color");
  strcpy(shared_array[1].value,"blue");

  if (fork()==0) {
    printf("CHILD\n");
    printf("The key is: %s\n", shared_array[0].key);
    printf("The value is: %s\n", shared_array[0].value);
    printf("The key is: %s\n", shared_array[1].key);
    printf("The value is: %s\n", shared_array[1].value);
    strcpy(shared_array[0].value,"dog");
    strcpy(shared_array[1].value,"green");
    shmdt((void *) shared_array);
  }
  else {
    sleep(1);
    printf("The key is: %s\n", shared_array[0].key);
    printf("The value is: %s\n", shared_array[0].value);
    printf("The key is: %s\n", shared_array[1].key);
    printf("The value is: %s\n", shared_array[1].value);;
    shmdt((void *) shared_array);
    shmctl(shmid, IPC_RMID, 0);
  }
}
