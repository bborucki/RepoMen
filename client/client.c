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
#include "../lib/objectmap.h"

#define STRLEN 81

  struct Globals {
    char debug;
    char mv;
    unsigned char x;
    unsigned char y;
    char host[STRLEN];
    PortType port;
    Map *map;
    Cell *cell;
    Player *player;
    Gamestate *gamestate;
    //  ObjectMap *objmap;
  } globals;

typedef struct ClientState  {
  int data;
  Proto_Client_Handle ph;
} Client;

static int
clientInit(Client *C){
  bzero(C, sizeof(Client));

  globals.player = (Player *)malloc(sizeof(Player));
  globals.map = (Map *)malloc(sizeof(Map));
  globals.cell = (Cell *)malloc(sizeof(Cell));

  bzero(globals.player,sizeof(globals.player));
  bzero(globals.map,sizeof(globals.map));
  bzero(globals.cell,sizeof(globals.cell));

  globals.gamestate = gamestate_create();

  if (proto_client_init(&(C->ph))<0) {
    fprintf(stderr, "client: main: ERROR initializing proto system\n");
    return -1;
  }
  return 1;
}

static int
update_event_handler(Proto_Session *s, Proto_Client_Handle ch){
  printf("Update from the server!\n");  
  fprintf(stderr, "%s: called", __func__);
  return 1;
}

int 
startConnection(Client *C, char *host, PortType port, Proto_Client_MT_Handler h){
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

int 
doRPC(Client *C, char c) 
{
  Proto_Msg_Hdr hdr;
  int rc=-1;
  int x,y;
  int offset=0;

  Proto_Session *s;
  switch (c) {
  case 'h':
    rc = proto_client_hello(C->ph);
    s = proto_client_rpc_session(C->ph);
    proto_session_hdr_unmarshall(s,&hdr);
    if(s->rhdr.pstate.v0.raw){
      globals.x = (unsigned char)s->rhdr.pstate.v1.raw;
      globals.y = (unsigned char)s->rhdr.pstate.v2.raw;
      offset = proto_session_body_unmarshall_gamestate(s,0,globals.gamestate);
      gamestate_dump(globals.gamestate);
      proto_session_body_unmarshall_map(s,offset,globals.map);
    }
    break;
  case 'i':
    rc = proto_client_cinfo(C->ph, globals.x, globals.y);
    s = proto_client_rpc_session(C->ph);
    proto_session_body_unmarshall_cell(s, 0, globals.cell);
    break;
  case 'v':
    //currently no validity checking client-side
    rc = proto_client_move(C->ph, globals.player->id, globals.mv);
    s = proto_client_rpc_session(C->ph);
    if(s->rhdr.pstate.v3.raw > 0){
      if(s->rhdr.pstate.v0.raw == globals.player->id){
	globals.x = s->rhdr.pstate.v1.raw;
	globals.y = s->rhdr.pstate.v2.raw;
	printf("Now at (%d,%d)\n", globals.x, globals.y);

	//insert way of blocking for a server move update
	//hmmmm
      }
    }
    else{
      printf("Invalid move\n");
    }
    break;
  case 'd':
    rc = proto_client_dump(C->ph);
    break;
  case 'g':
    rc = proto_client_goodbye(C->ph,globals.player);
    break;
  default:
    printf("%s: unknown command %c\n", __func__, c);
  }
  if (rc == 0xdeadbeef) rc=1;
  return rc;
}

int
doMove(Client *C){
  int rc,x;
  char ch;

  if((ch=getchar())=='\n'){
    printf("Failed to move, usage move <1/2/3/4>\n");
    return 1;
  }

  putchar(ch);
  scanf("%c", &ch);

  if(!proto_client_get_connected(C->ph)){
    printf("Not Connected.");
    return 1;
  }

  x = atoi(&ch)-1;

  if(x < 0 || x > 3){
    printf("Failed to move, usage move <1/2/3/4>\n");
    return 1;
  }

  globals.mv = x;
  
  rc = doRPC(C, 'v');

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

  if(proto_client_get_connected(C->ph)){
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
    doRPC(C,'h');
    printf("Connected.");
    printf("\n");
    printf("Location: (%d,%d)\n", globals.x, globals.y);
    player_dump(globals.player);
    proto_client_set_connected(C->ph, 1);
  }
  return 1;
}

extern int
doDisconnect(Client* C, char cmd){
  Proto_Session *rpc;
  Proto_Session *event;
  
  rpc = proto_client_rpc_session(C->ph);
  event = proto_client_event_session(C->ph);

  if(cmd == 'q' && !proto_client_get_connected(C->ph)){
    printf("Terminated.\n");
    return -1;
  }

  if(!proto_client_get_connected(C->ph)){
    printf("Not Connected.");
    return 1;
  }

  doRPC(C,'g'); //goodbye

  net_close_socket(rpc->fd);
  net_close_socket(event->fd);

  proto_client_set_connected(C->ph, 0);

  if(cmd=='q'){
    printf("Terminated.\n");
    return -1;
  }
  printf("Disconnected");
  return 1;
}

int
doWhere(Client *C){
  if(proto_client_get_connected(C->ph))
    printf("Connected to: %s:%d", globals.host, globals.port);
  else
    printf("Not Connected.");
  return 1;
}

int 
doNumHome(Client *C){
  char c;
  int teamNum;
  int rc;

  if((c=getchar())=='\n'){
    printf("Usage: \"numhome <1 or 2>\"\n");
    return 1;
  }    
  putchar(c);
  
  if(scanf("%d", &teamNum)==1 && (teamNum == 1 || teamNum == 2)){
    if(!proto_client_get_connected(C->ph)){
      printf("Not Connected.");
      return 1;
    }
    printf("Number of home cells for team %d = ", teamNum);
    if(teamNum == 1)
      printf("%d\n",globals.map->numhome1);
    else
      printf("%d\n",globals.map->numhome2);
  } else {
    printf("Usage: \"numhome <1 or 2>\"\n");
    return 1;
  }

  return 1;
}

int 
doNumJail(Client *C){
  char c;
  int teamNum;

  if((c=getchar())=='\n'){
    printf("Usage: \"numjail <1 or 2>\"\n");
    return 1;
  }
  putchar(c);
  
  if(scanf("%d", &teamNum)==1 && (teamNum == 1 || teamNum == 2)){
    if(!proto_client_get_connected(C->ph)){
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
doNumFloor(Client *C){
  if(!proto_client_get_connected(C->ph)){
    printf("Not Connected.");
    return 1;
  }
  printf("Number of available floor cells = %d\n", globals.map->numfloor);
  return 1;
}

int
doNumWall(Client *C){
  if(!proto_client_get_connected(C->ph)){
    printf("Not connected.");
    return 1;
  }    
  printf("Number of available wall cells = %d\n", globals.map->numwall);
  return 1;
}

int
doDim(Client *C){
  if(!proto_client_get_connected(C->ph)){
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
    if(!proto_client_get_connected(C->ph)){
      printf("Not Connected.");
      return 1;
    }        
    
    globals.x = x;
    globals.y = y;
    rc = doRPC(C,'i');
    printf("Type: ");
    cell_print_type(globals.cell);
    printf("Team: %d\n", globals.cell->team); //FIX ME
    if(globals.cell->player == NULL)
      printf("Unoccupied\n");
    else
      printf("Occupied by player: %d\n", globals.cell->player->id);
    printf("x: %d\n", globals.cell->x);
    printf("y: %d\n", globals.cell->y);
    printf("object1: %d\n", globals.cell->obj);

  } 
  else
    printf("Usage: \"cinfo <x,y>\"\n");
  
  return 1;
}

int 
doDump(Client *C){
  int rc;

  if(!proto_client_get_connected(C->ph)){
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
  printf("Available commands: \n\tconnect <IP:PORT> \n\tdisconnect \n\twhere \n\tnumhome <1 or 2> \n\tnumjail <1 or 2> \n\tnumwall \n\tnumfloor \n\tdim \n\tmove <1/2/3/4> \n\tdump \n\tquit\n");

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
  //  else if(!strcmp(input, "cinfo"))
  //    cmd = 'i';
  else if(!strcmp(input, "dump"))
    cmd = 'u';
  else if(!strcmp(input, "move"))
    cmd = 'v';
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
    rc=doWhere(C); 
    break;
  case 'q':
    rc=doDisconnect(C,cmd);
    break;
  case 'h': 
    rc=doNumHome(C);
    break;
  case 'j': 
    rc=doNumJail(C);
    break;
  case 'f': 
    rc = doNumFloor(C);
    break;
  case 'a': 
    rc = doNumWall(C);
    break;
  case 'm': 
    rc = doDim(C);
    break;
  case 'i': 
    rc = doCInfo(C);
    break;
  case 'u': 
    rc = doDump(C);
    break;
  case 'v': 
    rc = doMove(C);
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
