#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "display.h"
#include <stdio.h>

int main()
{
  struct sembuf upA = {0,1,0};
  struct sembuf downA = {0,-1,0};

  struct sembuf upB = {1,1,0};
  struct sembuf downB = {1,-1,0};

  int semaphore = semget(IPC_PRIVATE,2,0600);

  semop(semaphore,&upA,1); //Starting the semaphore in 1

  int i;
  if (fork()){

    for (i=0;i<10;i++){
      semop(semaphore,&downA,1);
      display("ab");
      semop(semaphore,&upB,1);
    }
    wait(NULL);
  }else{
    for (i=0;i<10;i++){
      semop(semaphore,&downB,1);
      display("cd\n");
      semop(semaphore,&upA,1);
    }
  }

  semctl(semaphore,0,IPC_RMID); //Destroy semaphore
  semctl(semaphore,1,IPC_RMID); //Destroy semaphore
  return 0;
}
