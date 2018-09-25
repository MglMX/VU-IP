#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define READ_END 0
#define WRITE_END 1

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

void print_params(char * params[],int size){
  int i;
  printf("Params:[");
  for(i=0;i<size;i++){
    printf("%s,",params[i]);
  }
  printf("]\n");
}

int count_vars(char * command){
  int count=0;
  int i;
  for(i=0;i<strlen(command);i++){
    if(command[i] == '|'){
      count++;
    }
  }

  return count;
}

void execute_program(char * params[]){
  pid_t pid;
  int stat;

  pid = fork();
  if(pid<0){
    printf("Fork error\n");
  }else if(pid == 0){
    execvp(params[0],params);
    exit(1);
    perror("Error executing exec\n");
  }else{
    waitpid(pid,&stat,0);
    //sleep(4);
  }
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


void pipe_commands(char * command_list,int n_commands){

  int stat;

  char * command;

  command = strtok(command_list,"|");

  char * command_array[n_commands];
  int count = 0;

  while(command != NULL){
    command_array[count]=command;
    command = strtok(NULL, "|");
    count++;
  }

  int n_pipes = n_commands -1;

  int fd[n_pipes][2];

  int i;
  for(i=0;i<n_pipes;i++){
    if(pipe(fd[i])<0){
      printf("Error creating pipe\n");
      exit(1);
    }
  }

  pid_t pid,first_pid,last_pid;

  for(i=0;i<n_commands;i++){

    if(i==0){//First one
      first_pid = fork();
      if(first_pid==0){
        int n_params = count_spaces(command_array[i])+1; //Program name and NULL
        char * params[n_params];
        make_param_vector(command_array[i],params,n_params);
        //print_params(params,n_params);

        dup2(fd[0][WRITE_END],STDOUT_FILENO);
        int i;
        for(i=0;i<n_pipes;i++){
          close(fd[i][READ_END]);
          close(fd[i][WRITE_END]);
        }
        execvp(params[0],params);
        exit(1);
      }

    }else if(i == n_pipes){//Last one
      last_pid = fork();
      if(last_pid==0){
        int n_params = count_spaces(command_array[i])+1; //Program name and NULL
        char * params[n_params];
        make_param_vector(command_array[i],params,n_params);

        dup2(fd[i-1][READ_END],STDIN_FILENO);
        int i;
        for(i=0;i<n_pipes;i++){
          close(fd[i][READ_END]);
          close(fd[i][WRITE_END]);
        }
        execvp(params[0],params);
        exit(1);
      }
    }else{//Middle ones
      pid = fork();
      if(pid==0){
        int n_params = count_spaces(command_array[i]); //Program name and NULL
        char * params[n_params];
        make_param_vector(command_array[i],params,n_params);


        dup2(fd[i-1][READ_END],STDIN_FILENO);
        dup2(fd[i][WRITE_END],STDOUT_FILENO);
        int i;
        for(i=0;i<n_pipes;i++){
          close(fd[i][READ_END]);
          close(fd[i][WRITE_END]);
        }
        execvp(params[0],params);
        exit(1);
      }
    }
  }//end for

  for(i=0;i<n_pipes;i++){
    close(fd[i][READ_END]);
    close(fd[i][WRITE_END]);
  }

  for(i=0;i<n_commands;i++){
    pid = wait(&stat);
  }
}


void ask_command(char * command){
  printf("$ ");
  fgets(command,100,stdin);
  command[strlen(command)-1] = '\0';
}


int main(){

  char *home;

  while(1){

    char command[1024];

    ask_command(command);

    int n_commands = count_vars(command)+1;
    int size_params = count_spaces(command)+2; //Two extra for program anme and NULL

    if(n_commands>1){
      pipe_commands(command,n_commands);
    }else{
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

      }else{
        execute_program(params);
      }
    }//n_commands == 1
  }//while
}
