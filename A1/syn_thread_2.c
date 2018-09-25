#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "display.h"
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex;
pthread_cond_t cond_var;

int predicateA = 1;
int predicateB = 0;

void * print_ab(){
  int i;
  for (i=0;i<10;i++){
    pthread_mutex_lock(&mutex);
    while(predicateA == 0)
      pthread_cond_wait(&cond_var,&mutex);
    predicateA = 0;
    pthread_mutex_unlock(&mutex);

    display("ab");

    pthread_mutex_lock(&mutex);
    predicateB = 1;
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&mutex);
  }
}

void * print_cd(){
  int i;
  for (i=0;i<10;i++){
    pthread_mutex_lock(&mutex);
    while(predicateB == 0)
      pthread_cond_wait(&cond_var,&mutex);
    predicateB = 0;
    pthread_mutex_unlock(&mutex);

    display("cd\n");

    pthread_mutex_lock(&mutex);
    predicateA = 1;
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&mutex);
  }
}

int main()
{
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_var, NULL);

  pthread_t id_ab, id_cd;

  pthread_create(&id_ab,NULL,print_ab,NULL);
  pthread_create(&id_cd,NULL,print_cd,NULL);

  pthread_join(id_ab,NULL);
  pthread_join(id_cd,NULL);

  return 0;
}
