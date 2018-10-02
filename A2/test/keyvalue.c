#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "keyvalue.h"

struct pair{
  char key[50];
  char value[50];
  int sem;
};

int *size;
int sem_new_put;
struct sembuf up = {0,1,0};
struct sembuf down = {0,-1,0};
//struct pair dictionary[1000];

struct pair *dictionary;
int shmid;
int shmid_size;

void init_array(int length){
  shmid = shmget(IPC_PRIVATE, sizeof(struct pair)*length, 0600);
  shmid_size = shmget(IPC_PRIVATE, sizeof(int), 0600);

  dictionary = (struct pair *) shmat(shmid, 0, 0);
  size = (int *) shmat(shmid_size, 0, 0);

  *size = 0;
  sem_new_put = semget(IPC_PRIVATE,1,0600);
  semop(sem_new_put,&up,1); //Starting the semaphore in 1

}

void print_all(){
  int i;
  for(i=0;i<*size;i++){
    printf("dictionary[%d].key: %s\n",i,dictionary[i].key);
    printf("dictionary[%d].value: %s\n",i,dictionary[i].value);
  }
}

char *get(char *key){
    //printf("In get. key: %s size: %d\n",key,size);
    int i;
    for(i=0;i<*size;i++){
      //printf("In get. Checking dictionary[%d].key: %s\n",i,dictionary[i].key);
      if(strcmp(dictionary[i].key,key)==0){
        //printf("In get. value: %s\n",dictionary[i].value);
        return dictionary[i].value;
      }
    }

    return NULL;
}

void put(char *key, char *value){
  int found = 0;
  int i;
  for(i=0;i<*size && !found;i++){
      if(strcmp(dictionary[i].key,key)==0){
        semop(dictionary[i].sem,&down,1);
        strcpy(dictionary[i].value,value);
        found = 1;
        semop(dictionary[i].sem,&up,1);
      }
  }

  if(!found){ //Adding new pair
    semop(sem_new_put,&down,1); //Locking so there are no problems with size
    strcpy(dictionary[*size].key,key);
    strcpy(dictionary[*size].value,value);
    dictionary[*size].sem = semget(IPC_PRIVATE,1,0600); //Creating lock for the element
    semop(dictionary[*size].sem,&up,1); //Make it accesible
    *size+=1;
    semop(sem_new_put,&up,1); //Unlocking
  }
}

void dettach_mem(){
  shmdt((void *) dictionary);
}

void ctl_mem(){
  shmctl(shmid, IPC_RMID, 0);
}
