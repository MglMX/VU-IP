#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int count_spaces(char * command){
  int count=0;
  int i;
  int inside_quotes=0;
  for(i=0;i<strlen(command);i++){
    if(command[i] == '"'){
      inside_quotes = !inside_quotes;
    }
    if(!inside_quotes && command[i] == ' '){
      count++;
    }
  }

  return count;
}

void make_param_vector(char * command, char * params[], int size){
  char * param;

  param = strtok(command," ");

  int count = 0;
  while( param != NULL ) {
    params[count]=param;
    param = strtok(NULL, " ");
    count++;
  }

  params[size-1]= NULL;
}

void print_params(char * params[],int size){
  int i;
  printf("Params:[");
  for(i=0;i<size;i++){
    printf("%s,",params[i]);
  }
  printf("]\n");
}

void ask_command(char * command){
  printf("$ ");
  fgets(command,100,stdin);
  command[strlen(command)-1] = '\0';
}


int main(){

  pid_t pid;
  int stat;

  char *home;

  while(1){

    char command[1024];


    ask_command(command);


    int size_params = count_spaces(command)+2; //Two extra for program anme and NULL

    //printf("Size_params: %d\n",size_params);

    char * params[size_params];

    make_param_vector(command,params,size_params);

    if(strcmp(params[0],"exit")==0){
      break;
    }else if(strcmp(params[0],"cd")==0){

      if(size_params==2){//Only program name and null
        home = getenv("HOME");
        chdir(home);
      }else{
        chdir(params[1]);
      }
      //Needs to be changed

    }else{

      pid = fork();
      if(pid<0){
        printf("Fork error\n");
      }else if(pid == 0){
      	char * null_vector[2];
      	null_vector[0]=params[0];
      	null_vector[1]=NULL;
        execvp(params[0],null_vector);
        exit(1);
        perror("Error executing exec\n");
      }else{
        waitpid(pid,&stat,0);
        //sleep(4);
      }
    }
  }
}
