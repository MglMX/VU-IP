#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "display.h"
#include <stdio.h>


int main()
{


  struct sembuf up = {0,1,0};
  struct sembuf down = {0,-1,0};

  int semaphore = semget(IPC_PRIVATE,1,0600);

  semop(semaphore,&up,1); //Starting the semaphore in 1


  /*
  int v1 = semctl(semaphore,0,GETVAL);

  printf("v1: %d\n",v1);

  semop(semaphore,&up,1);

  int v2 = semctl(semaphore,0,GETVAL);

  printf("After up v2: %d\n",v2);

  semop(semaphore,&down,1);

  int v3 = semctl(semaphore,0,GETVAL);

  printf("After down v3: %d\n",v3);

  semctl(semaphore,0,IPC_RMID);

  int v4 = semctl(semaphore,0,GETVAL);

  printf("After destryo v4: %d\n",v4);
  */

  int i;
  if (fork())
  {
    for (i=0;i<10;i++){
      semop(semaphore,&down,1);
      display("Hello world\n");
      semop(semaphore,&up,1);
    }
    wait(NULL);
  }
  else
  {
    for (i=0;i<10;i++){
      semop(semaphore,&down,1);
      display("Bonjour monde\n");
      semop(semaphore,&up,1);
    }
  }

  semctl(semaphore,0,IPC_RMID); //Destroy semaphore
  return 0;
}
