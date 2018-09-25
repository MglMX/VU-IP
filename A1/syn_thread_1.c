#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "display.h"
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex;

void * print_english(){
  int i;
  for (i=0;i<10;i++){
    pthread_mutex_lock(&mutex);
    display("Hello world\n");
    pthread_mutex_unlock(&mutex);
  }
}

void * print_french(){
  int i;
  for (i=0;i<10;i++){
    pthread_mutex_lock(&mutex);
    display("Bonjour monde\n");
    pthread_mutex_unlock(&mutex);
  }
}

int main()
{

  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutex_init(&mutex,&attr);

  pthread_t id_en, id_fr;

  pthread_create(&id_en,NULL,print_english,NULL);
  pthread_create(&id_fr,NULL,print_french,NULL);

  pthread_join(id_en,NULL);
  pthread_join(id_fr,NULL);
  return 0;
}
