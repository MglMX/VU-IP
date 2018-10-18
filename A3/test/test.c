#include <stdio.h>
#include <string.h>

void printer(char * m, int size){
  int i;

  for(i=0;i<size;i++){
    if(i == 0){
      printf("%d",m[i]);
    }else{
      if(m[i]=='\0')
        printf("\\0");
      else
        printf("%c",m[i]);
    }

  }
  printf(" (%d) \n",size);
}

void printer_index(char * m, int size){
  int i;

  for(i=0;i<size;i++){
    if(i == 0){
      printf("0. %d\n",m[i]);
    }else{
      if(m[i]=='\0')
        printf("%d \\0\n",i);
      else
        printf("%d %c\n",i,m[i]);
    }

  }
  printf(" (%d) \n",size);
}

void get_ip_port2(char *message,char * ip, char * port){

  int pos=1;
  strcpy(ip,&message[1]);

  printf("1.IP: %s\n",ip);
  printer_index(ip,20);

  pos+=strlen(ip)+1;
  strcpy(port,&message[pos]);

  printer_index(ip,20);

  printf("2.IP: %s\n",ip);
  printf("PORT: %s\n",port);
}

void get_ip_port(char * ip, char * port){

  char message[16];

  message[0]=21;
  strcpy(&message[1],"127.0.0.1");
  printf("IP: %s\n",&message[1]);
  strcpy(&message[11],"7777");

  //memset(ip,'\0',20);
  //memset(port,'\0',4);

  //printer(message,16);

  //char ip[20];
  //char port[4];

  strcpy(ip,&message[1]);

  printf("1.IP: %s\n",ip);

  strcpy(port,&message[11]);

  printf("2.IP: %s\n",ip);
  printf("PORT: %s\n",port);
}

/*
void send_q_ok(int server_fd,char * ip, char * port){

  char reply[100];

  printf("strlen(reply): %zu\n",strlen(reply));
  memset(reply,'\0',100);

  reply[0]=21; //Q_OK code

  printf("Send_q_ok: IP: %s PORT: %s \n",ip,port);

  int pos=1;
  strcat(&reply[pos],ip); //21<ip>
  pos += strlen(ip);
  reply[pos]='\0'; //21<ip>'\0'
  pos+=1;
  strcat(&reply[pos],port);
  pos+=strlen(port);
  reply[pos]='\0'; //21<ip>\0<port>\0
  pos+=1;

  printer(reply,pos);

  writen(server_fd,reply,pos);
}
*/
int main(){

  char message[8];
  memset(message,'\0',8);

  int pos=0;
  strcpy(message,"abc");
  pos+=strlen("abc"); //abc_ (pos: 3)
  message[pos]='\0';
  pos+=1; //abc\0_ (pos:4)
  strcpy(&message[pos],"def");
  pos+=strlen("def"); //abc\0def_
  message[pos]='\0';
  pos+=1;

  printf("pos: %d\n",pos);
  printf("message: %s(%zu)\n",message,strlen(message));
  printer(message,pos);

  char str1[10];
  char str2[10];

  pos=0;

  sprintf(str1,"%s",&message[pos]);
  printf("str1: %s(%zu)\n",str1,strlen(str1));

  pos+=strlen(str1)+1;

  sprintf(str2,"%s",&message[pos]);
  printf("str2: %s(%zu)\n",str2,strlen(str2));

  /*
  int str1_size = strlen(&message[pos]);

  printf("str1_size: %d\n",str1_size);

  strncpy(str1,&message[pos],str1_size);

  str1[str1_size]='\0';
  printf("str1: %s\n",str1);

  pos+=str1_size+1;
  int str2_size = strlen(&message[pos]);
  strncpy(str2,&message[pos],str2_size);
  str2[str2_size]='\0';
  */
  printf("str1: %s\nstr2: %s\n",str1,str2);


}
