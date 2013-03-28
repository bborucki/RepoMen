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
#include "../lib/protocol_session.h"
#include "../lib/protocol_utils.h"
#include "../lib/map.h"
#include "../lib/cell.h"

#define STRLEN 81

struct Globals {
  char host[STRLEN];
  PortType port;
  char connected;
  int x;
  int y;
  Map *map;
  Cell *cell;
  char debug;
} globals;


typedef struct ClientState  {
  int data;
  Proto_Client_Handle ph;
} Client;

static int
clientInit(Client *C){
  bzero(C, sizeof(Client));

  if (proto_client_init(&(C->ph))<0) {
    fprintf(stderr, "client: main: ERROR initializing proto system\n");
    return -1;
  }
  return 1;
}

static int
update_event_handler(Proto_Session *s){
  Client *C = proto_session_get_data(s);

  fprintf(stderr, "%s: called", __func__);
  return 1;
}

int 
startConnection(Client *C, char *host, PortType port, Proto_MT_Handler h){
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
  Proto_Session *s;
  switch (c) {
  case 'h':
    rc = proto_client_hello(C->ph);
    printf("hello: rc=%x\n", rc);
    if (rc > 0) game_process_reply(C);
    break;
  case 'q':
    rc = proto_client_query(C->ph);
    s = proto_client_rpc_session(C->ph);
    globals.map = malloc(sizeof(Map));
    proto_session_body_unmarshall_map(s,0,globals.map);
    break;
  case 'i':
    rc = proto_client_cinfo(C->ph, globals.x, globals.y);
    s = proto_client_rpc_session(C->ph);
    globals.cell = malloc(sizeof(Cell));
    proto_session_body_unmarshall_cell(s, 0, globals.cell);
    break;
  case 'd':
    rc = proto_client_dump(C->ph);
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
  //  printf("%s: rc=0x%x\n", __func__, rc);
  if (rc == 0xdeadbeef) rc=1;
  return rc;
}

int
doRPC(Client *C, char cmd)
{
  int rc;

  rc=doRPCCmd(C,cmd);

  //  printf("doRPC: rc=0x%x\n", rc);

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

  if(globals.connected){
    printf("Already Connected.");
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
doDisconnect(Client *C, char cmd){
  if(cmd == 'q' && !globals.connected){
    printf("Terminated.\n");
    return -1;
  }

  if(!globals.connected){
    printf("Not Connected.");
    return 1;
  }

  doRPC(C,'g'); //goodbye

  globals.connected = 0;

  if(cmd=='q'){
    printf("Terminated.\n");
    return -1;
  }
  printf("Disconnected");
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
    if(!globals.connected){
      printf("Not Connected.");
      return 1;
    }
    printf("Number of home cells for team %d = ", teamNum);
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
    if(!globals.connected){
      printf("Not Connected.");
      return 1;
    }
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
  if(!globals.connected){
    printf("Not Connected.");
    return 1;
  }    
  printf("Number of available floor cells = %d\n", globals.map->numfloor);
  return 1;
}

int
doNumWall(){
  if(!globals.connected){
    printf("Not connected.");
    return 1;
  }    
 printf("Number of available wall cells = %d\n", globals.map->numwall);
 return 1;
}

int
doDim(){
  if(!globals.connected){
    printf("Not Connected.");
    return 1;
  }    
  printf("Dimensions of the maze are %d x %d\n", globals.map->dim-1,globals.map->dim-1);
  return 1;
}

int 
doCInfo(Client *C){
  char c;
  int rc,x,y;

  if((c=getchar())=='\n'){
    printf("Usage: \"cinfo <x,y>\"\n");
    return 1;
  }
  putchar(c);

  if(scanf("%d,%d", &x, &y)==2){
    if(!globals.connected){
      printf("Not Connected.");
      return 1;
    }        

    globals.x = x;
    globals.y = y;
    rc = doRPC(C,'i');
    printf("Type: ");
    cell_print_type(globals.cell);
    printf("Team: %d\n", globals.cell->team);
    printf("Occupied: %d\n", globals.cell->occupied);
    printf("x: %d\n", globals.cell->x);
    printf("y: %d\n", globals.cell->y);
    printf("object1: %d\n", globals.cell->obj1);
    printf("object2: %d\n", globals.cell->obj2);


  }
  else{
    printf("Usage: \"cinfo <x,y>\"\n");
    return 1;
  }
    
  return 1;
}

int 
doDump(Client *C){
  int rc;

  if(!globals.connected){
    printf("Not Connected.");
    return 1;
  }
  
  if((rc=doRPC(C,'d'))>0){
    printf("Dump Successful\n");
    return 1;
  }

  return rc;
}

int
doHelp(){
  printf("Available commands: \n\tconnect <IP:PORT> \n\tdisconnect \n\twhere \n\tnumhome <1 or 2> \n\tnumjail <1 or 2> \n\tnumwall \n\tnumfloor \n\tdim \n\tcinfo <x,y> \n\tdump \n\tquit\n");

  return 1;
}

int
doDebug(){
  globals.debug = !globals.debug;
  if(globals.debug)
    printf("Debugging Mode: ON\n");
  else
    printf("Debugging Mode: OFF\n");
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
  else if(!strcmp(input, "debug"))
    cmd = 'b';
  else if(!strcmp(input, "help"))
    cmd = 'p';
    
  return cmd;
}

int 
docmd(Client *C, char cmd)
{
  int rc = 1;

  switch (cmd) {
  case 'c': 
    rc=doConnect(C);
    break;
  case 'd': 
    rc=doDisconnect(C,cmd);
    break;
  case 'w': 
    rc=doWhere(); 
    break;
  case 'q': 
    rc=doDisconnect(C,cmd);
    break;
  case 'h': 
    rc=doNumHome();
    break;
  case 'j': 
    rc=doNumJail();
    break;
  case 'f': 
    rc = doNumFloor();
    break;
  case 'a': 
    rc = doNumWall();
    break;
  case 'm': 
    rc = doDim();
    break;
  case 'i': 
    rc = doCInfo(C);
    break;
  case 'u': 
    rc = doDump(C);
    break;
  case 'b': 
    rc = doDebug();
    break;
  case 'p': 
    rc = doHelp();
    break;
  case ' ':
  case '\n':
    rc=1;
    break;
  default:
    printf("Unkown Command\n");
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
