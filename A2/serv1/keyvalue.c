#include "keyvalue.h"
#include <string.h>
#include <stdio.h>

struct pair{
  char key[50];
  char value[50];
};

int size = 0;

struct pair dictionary[1000];

void print_all(){
  int i;
  for(i=0;i<size;i++){
    printf("dictionary[%d].key: %s\n",i,dictionary[i].key);
    printf("dictionary[%d].value: %s\n",i,dictionary[i].value);
  }
}

char *get(char *key){
    //printf("In get. key: %s size: %d\n",key,size);
    int i;
    for(i=0;i<size;i++){
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
  for(i=0;i<size && !found;i++){
      if(strcmp(dictionary[i].key,key)==0){
        strcpy(dictionary[i].value,value);
        found = 1;
      }
  }

  if(!found){
    strcpy(dictionary[size].key,key);
    strcpy(dictionary[size].value,value);
    size++;
  }
}
