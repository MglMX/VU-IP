while(argv[arg_check] != NULL){

  printf("I am in the while\n");

  if(strcmp(argv[arg_check],"get")==0){
      int msg_pos = s_pos;
      char * key = argv[arg_check+1]; //After get we get the key

      if(key == NULL){
        perror("Wrong syntax\n");
        exit(1);
      }

      printf("Key_ %s\n",key);

      if(arg_check == 3){ //Message not initalized
        strcpy(message,"g"); //Equivalent to 103
      }else{
        strcat(&message[s_pos],"g");
      }

      s_pos = s_pos + 1;//Next writting position g_

      strcat(&message[s_pos],key); //Concat the key to the message

      s_pos = s_pos + strlen(key); //Next writting position g<key>_

      message[s_pos]='\0'; //Add \0 to finish command

      printf("Message to be sent: %s(%zu)\n",&message[msg_pos],strlen(&message[msg_pos]));

      s_pos=s_pos+1; //Position where next command should start g<key>\0_
      arg_check+=2; // Next argument to check from argv

      printf("s_pos: %d\n",s_pos);

  }else if(strcmp(argv[arg_check],"put")==0){
      char * key = argv[arg_check+1]; //After put we get the key
      char * value = argv[arg_check+2]; //After key we get the value

      int msg_pos = s_pos;

      if(key == NULL || value == NULL){
        perror("Wrong syntax\n");
        exit(1);
      }
      printf("Key_ %s\nValue_ %s\n",key,value);

      if(arg_check == 3){ //Message not initalized
        strcpy(message,"p"); //Equivalent to 103
      }else{
        strcat(&message[s_pos],"p");
      }

      s_pos+= 1;//Next writting position p_

      strcat(&message[s_pos],key); //Concat the key to the message

      s_pos+= strlen(key); //Next writting position p<key>_

      message[s_pos]='\0'; //Add \0 to indicate end of Key_

      s_pos+= 1; //Next writting position p<key>\0_

      strcat(&message[s_pos],value); //Concat the value to the message

      s_pos+=strlen(value); //p<key>\0<value>_

      message[s_pos]='\0'; //Add \0 to finish command

      printf("Message to be sent: %s(%zu)\n",&message[msg_pos],strlen(&message[msg_pos]));

      s_pos+= 1; //Position where next command should start p<key>\0<value>\0_
      arg_check+=3; // Next argument to check from argv

  }else{
    //Wrong syntax
  }
}


///////////////////////////////////////////////////////////

















printf("I am in put\n");
char * key = argv[arg_check+1]; //After put we get the key
char * value = argv[arg_check+2]; //After key we get the value

printf("Key_ %s\nValue_ %s\n",key,value);

if(arg_check == 3){ //Message not initalized
  strcpy(message,"p"); //Equivalent to 103
}else{
  strcat(&message[s_pos],"p");
}

strcat(&message[s_pos+1],key); //Concat the key to the message
message[s_pos+strlen(key)+1]='\0'; //Add \0 to finish command
