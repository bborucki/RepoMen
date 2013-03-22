/******************************************************************************
* Copyright (C) 2011 by Jonathan Appavoo, Boston University
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/types.h"
#include "../lib/protocol_client.h"
#include "../lib/protocol_utils.h"
#include "../lib/map.h"

#define STRLEN 81

struct Globals {
  char host[STRLEN];
  PortType port;
  char connected;
  Map* map;
} globals;


typedef struct ClientState  {
  int data;
  Proto_Client_Handle ph;
} Client;

static int
clientInit(Client *C)
{
  bzero(C, sizeof(Client));

  // initialize the client protocol subsystem
  if (proto_client_init(&(C->ph))<0) {
    fprintf(stderr, "client: main: ERROR initializing proto system\n");
    return -1;
  }
  return 1;
}

static int
update_event_handler(Proto_Session *s)
{
  Client *C = proto_session_get_data(s);

  fprintf(stderr, "%s: called", __func__);
  return 1;
}


int 
startConnection(Client *C, char *host, PortType port, Proto_MT_Handler h)
{
  if (globals.host[0]!=0 && globals.port!=0) {
    if (proto_client_connect(C->ph, host, port)!=0) {
      fprintf(stderr, "failed to connect\n");
      return -1;
    }
   proto_session_set_data(proto_client_event_session(C->ph), C);
#if 1
    if (h != NULL) {
      proto_client_set_event_handler(C->ph, PROTO_MT_EVENT_BASE_UPDATE, 
				     h);
    }
#endif
    return 1;
  }
  return 0;
}

// FIXME:  this is uly maybe the speration of the proto_client code and
//         the game code is dumb
int
game_process_reply(Client *C)
{
  Proto_Session *s;

  s = proto_client_rpc_session(C->ph);

  fprintf(stderr, "%s: do something %p\n", __func__, s);

  return 1;
}

int 
doRPCCmd(Client *C, char c) 
{
  int rc=-1;

  switch (c) {
  case 'h':  
    {
      rc = proto_client_hello(C->ph);
      printf("hello: rc=%x\n", rc);
      if (rc > 0) game_process_reply(C);
    }
    break;
  case 'q':
    rc = proto_client_query(C->ph);
    break;
  case 'f':
    printf("TIME TO PARTY!!!!\n");
    rc = 1;
    break;
  case 'm':
    scanf("%c", &c);
    rc = proto_client_move(C->ph, c);
    break;
  case 'g':
    rc = proto_client_goodbye(C->ph);
    break;
  default:
    printf("%s: unknown command %c\n", __func__, c);
  }
  // NULL MT OVERRIDE ;-)
  printf("%s: rc=0x%x\n", __func__, rc);
  if (rc == 0xdeadbeef) rc=1;
  return rc;
}

int
doRPC(Client *C, char cmd)
{
  int rc;
  //  char c;

  /*
    printf("enter (h|m<c>|g): ");
    scanf("%c", &c);
    rc=doRPCCmd(C,c);
  */

  rc=doRPCCmd(C,cmd);

  printf("doRPC: rc=0x%x\n", rc);

  return rc;
}

int
doConnect(Client *C){
  char addr_port[42];
  char host[81];
  int port;
  char ch;
  char *ptr;

  if((ch=getchar())=='\n'){
    printf("Failed to connect. Usage: \"connect <IP:PORT>\"\n");
    return 1;
  }
  putchar(ch);
  scanf("%s", addr_port);
  
  if((ptr = strpbrk(addr_port, ":"))==NULL){
    printf("Failed to connect. Usage: \"connect <IP:PORT>\"\n");
    return 1;
  }
  
  *ptr = '\0';
  
  strncpy(globals.host, addr_port, strlen(addr_port));
  globals.port = atoi(ptr+sizeof(char));

  if (startConnection(C, globals.host, globals.port, update_event_handler)<0) {
    fprintf(stderr, "ERROR: startConnection failed\n");
    return -1;
  }
  else{
    printf("Connected.");
    globals.connected = 1;
    doRPC(C,'q');

  }
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
  if(globals.connected)
    printf("Connected to: %s:%d", globals.host, globals.port);
  else
    printf("Not Connected.");
  return 1;
}

int 
doNumHome(){
  char c;
  int teamNum;
  int rc;

  if((c=getchar())=='\n'){
    printf("Usage: \"numhome <1 or 2>\"\n");
    return 1;
  }
  putchar(c);
  
  if(scanf("%d", &teamNum)==1 && (teamNum == 1 || teamNum == 2)){
    printf("number of home cells for team %d = ", teamNum);
    if(teamNum == 1)
      printf("%d\n",globals.map->numhome1);
    else
      printf("%d\n",globals.map->numhome2);
  }
  else{
    printf("Usage: \"numhome <1 or 2>\"\n");
    return 1;
  }

  return 1;
}

int 
doNumJail(){
  char c;
  int teamNum;

  if((c=getchar())=='\n'){
    printf("Usage: \"numjail <1 or 2>\"\n");
    return 1;
  }
  putchar(c);
  
  if(scanf("%d", &teamNum)==1 && (teamNum == 1 || teamNum == 2)){
    printf("Number of jail cells for team %d = ", teamNum);
    if(teamNum == 1)
      printf("%d\n",globals.map->numjail1);
    else
      printf("%d\n",globals.map->numjail2);

  }
  else{
    printf("Usage: \"numjail <1 or 2>\"\n");
    return 1;
  }

  return 1;
}

int
doNumFloor(){
  printf("Number of available floor cells = %d\n", globals.map->numfloor);
  return 1;
}

int
doNumWall(){
 printf("Number of available floor cells = %d\n", globals.map->numwall);
 return 1;
}

int
doDim(){
  printf("Dimensions of the maze are %d x %d\n", globals.map->dim,globals.map->dim);
  return 1;
}

int 
doCInfo(){
  char c;
  int x,y;

  if((c=getchar())=='\n'){
    printf("Usage: \"cinfo <x,y>\"\n");
    return 1;
  }
  putchar(c);

  if(scanf("%d,%d", &x, &y)==2){
    printf("Info for (%d,%d) = ...", x, y);
    //rc = doRPC(C,'i');
  }
  else{
    printf("Usage: \"cinfo <x,y>\"\n");
    return 1;
  }
    
  return 1;
}

int 
doDump(){
  int i;
  printf("dumping!");
  for(i = 0; i<COLUMN_MAX; i++)
    printf("%s",globals.map->maze[i]);
  return 1;
}

int
doHelp(){
  printf("Available commands: \n\tconnect <IP:PORT> \n\tdisconnect \n\twhere \n\tnumhome <1 or 2> \n\tnumjail <1 or 2> \n\tnumwall \n\tnumfloor \n\tdim \n\tcinfo <x,y> \n\tdump \n\tquit\n");

  return 1;
}

char
input2cmd(char* input){
  int cmd=-1;
 
  if(!strcmp(input, "connect"))
    cmd = 'c';
  else if(!strcmp(input, "disconnect"))
    cmd = 'd';
  else if(!strcmp(input, "where"))
    cmd = 'w';
  else if(!strcmp(input, "quit"))
    cmd = 'q';
  else if(!strcmp(input, "numhome"))
    cmd = 'h';
  else if(!strcmp(input, "numjail"))
    cmd = 'j';
  else if(!strcmp(input, "numfloor"))
    cmd = 'f';
  else if(!strcmp(input, "numwall"))
    cmd = 'a';
  else if(!strcmp(input, "dim"))
    cmd = 'm';
  else if(!strcmp(input, "cinfo"))
    cmd = 'i';
  else if(!strcmp(input, "dump"))
    cmd = 'u';
  else if(!strcmp(input, "help"))
    cmd = 'p';
    
  return cmd;
}

int 
docmd(Client *C, char cmd)
{
  int rc = 1;

  switch (cmd) {
  case 'c': //connect
    rc=doConnect(C);
    break;
  case 'd': //disconnect
    rc=doDisconnect();
    break;
  case 'w': //where
    rc=doWhere(); 
    break;
  case 'q': //quit
    rc=-1;
    break;
  case 'h': //numhome
    rc=doNumHome();
    break;
  case 'j': //numjail
    rc=doNumJail();
    break;
  case 'f': //numfloor
    rc = doNumFloor();
    break;
  case 'a': //numwall
    rc = doNumWall();
    break;
  case 'm': //dim
    rc = doDim();
    break;
  case 'i': //cinfo
    rc = doCInfo();
    break;
  case 'u': //dump
    rc = doDump();
    break;
  case 'p': //help
    rc = doHelp();
    break;
  case '\n':
    rc=1;
    break;
  default:
    printf("Unkown Command\n");
    /*
      case 'd':
      proto_debug_on();
      break;
      case 'D':
      proto_debug_off();
      break;    
      case 'r':
      rc = doRPC(C);
      break;
    */
  }
  return rc;
}

int
prompt(int menu) 
{
  static char MenuString[] = "\nclient> ";
  int c=0;
  char input[42];

  if (menu) printf("%s", MenuString);
  fflush(stdout);
  scanf("%s", input);

  c=input2cmd(input);

  return c;
}

void *
shell(void *arg)
{
  Client *C = arg;
  char c;
  int rc;
  int menu=1;

  while (1) {
    if ((c=prompt(menu))!=0) rc=docmd(C, c);
    if (rc<0) break;
    if (rc==1) menu=1; else menu=0;
  }

  fprintf(stderr, "terminating\n");
  fflush(stdout);
  return NULL;
}

int 
main(int argc, char **argv)
{
  Client c;

  bzero(&globals, sizeof(globals));

  if (clientInit(&c) < 0) {
    fprintf(stderr, "ERROR: clientInit failed\n");
    return -1;
  }    

  shell(&c);

  return 0;
}
