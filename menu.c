#include <stdio.h>
#include <string.h>

char
input2cmd(char* input){
  int cmd=-1;
  
  if(strlen(input)==1)
    return input[0];
 
  //still need to handle the case where
  //the input is an integer
 
  if(!strcmp(input, "connect"))
    cmd = 'c';
  if(!strcmp(input, "disconnect"))
    cmd = 'd';
  if(!strcmp(input, "where"))
    cmd = 'w';
  if(!strcmp(input, "quit"))
    cmd = 'q';
    
  return cmd;
}

char
prompt(int menu) 
{
  static char MenuString[] = "\nmenu> ";
  char c=0;
  char input[42];

  if (menu) printf("%s", MenuString);
  fflush(stdout);
  scanf("%s", input);

  c=input2cmd(input);

  return c;
}

int
doConnect(){
  char addr_port[42];
  char c;

  if((c=getchar())=='\n'){
    printf("Failed to connect. Usage: \"connect <IP:PORT>\"");
    return 1;
  }
  putchar(c);
  scanf("%s", addr_port);
  printf("Attempting to connect to : %s", addr_port);

  //parse the IP and PORT
  //attempt connect
  //get angry if fails
  
  return 1;
}

int
doDisconnect(){
  printf("Disconnecting...");
  //call terminating function...whatever that is
  return 1;
}

int
doWhere(){
  printf("Connected to:...");
  //return globally stored IP and PORT
  return 1;
}

int 
docmd(char cmd)
{
  int rc = 1;

  switch (cmd) {
  case 'c':
    rc=doConnect();
    break;
  case 'd':
    rc=doDisconnect();
    break;
  case 'w':
    rc=doWhere();
    break;
    //May also want to put debugging options back in
  case 'q':
    rc=-1;
    break;
  case '\n':
    rc=1;
    break;
  default:
    printf("Unkown Command\n");
  }
  return rc;
}

int
shell(void){
  char c;
  int rc;
  int menu=1;
  
  while (1) {
    if ((c=prompt(menu))!=0) rc=docmd(c);
    if (rc<0) break;
    if (rc==1) menu=1; else menu=0;
  }

  fprintf(stderr, "terminating\n");
  fflush(stdout);
}

int
main(int argc,char *argv){
  return shell();
}
