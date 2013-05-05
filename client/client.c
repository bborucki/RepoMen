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
  Cell *cell;
} globals;

struct ClientState {
  Proto_Client_Handle ph;
  Gamestate *Gamestate;
  Player *Player;
  Map *Map; 
  int connected;
  int data;
} Client;

int 
startConnection(char *host, PortType port, Proto_MT_Handler h){
  if (globals.host[0]!=0 && globals.port!=0) {
    if (proto_client_connect(Client.ph, host, port)!=0) {
      fprintf(stderr, "failed to connect\n");
      return -1;
    }
    proto_session_set_data(proto_client_event_session(Client.ph), NULL);
#if 1
    if (h != NULL) {
      proto_client_set_event_handler(Client.ph, PROTO_MT_EVENT_BASE_UPDATE, 
				     h);
    }
#endif
    return 1;
  }
  return 0;
}

static int
update_event_handler(Proto_Session *s){
  printf("Update from the server!\n");  
  fprintf(stderr, "%s: called", __func__);
  return 1;
}

static int 
proto_client_session_lost_default_hdlr(Proto_Session *s){
  fprintf(stderr, "Session lost...:\n");
  proto_session_dump(s);
  net_close_socket(s->fd);

  return -1;
}

static int 
event_null_handler(Proto_Session *s){
  //  fprintf(stderr,"proto_client_event_null_handler: invoked for session:\n");
  //  proto_session_dump(s);

  return 1;
}

static int 
event_player_quit_handler(Proto_Session *s){
  Player p;

  proto_session_body_unmarshall_player(s,0,&p);
  printf("Player %d has disconnected.\n", p.id);  

  //remove appropriate player from gamestate

  return 1;
}

static int 
event_player_join_handler(Proto_Session *s){
  int x,y,id,ret;
  Player *p;

  p = player_create();

  proto_session_hdr_unmarshall(s,&s->rhdr);

  id = s->rhdr.pstate.v0.raw;
  x = s->rhdr.pstate.v1.raw;
  y = s->rhdr.pstate.v2.raw;

  if((ret = proto_session_body_unmarshall_player(s,0,p)) == 0){
    fprintf(stderr, "Error unmarshalling new player.\n");
    return -1;
  }

  gamestate_add_player(Client.Gamestate,p);
  gamestate_dump(Client.Gamestate);

  return 1;
}

static int 
event_move_handler(Proto_Session *s){
  int x,y,id;

  proto_session_hdr_unmarshall(s,&s->rhdr);

  id = s->rhdr.pstate.v0.raw;
  x = s->rhdr.pstate.v1.raw;
  y = s->rhdr.pstate.v2.raw;

  gamestate_move_player(Client.Gamestate,id,x,y);
  //  gamestate_dump(Client.Gamestate);
  
  printf("Player %d is now at (%d,%d)\n", id, x, y);  
  
  return 1;
}

static int 
event_pickup_handler(Proto_Session *s){
  printf("proto_client_event_pickup_handler invoked!\n");  

  return 1;
}

static int 
event_drop_handler(Proto_Session *s){
  printf("proto_client_event_drop_handler invoked!\n");  

  return 1;
}

static int 
event_win_handler(Proto_Session *s){
  printf("proto_client_event_win_handler invoked!\n"); 

  return 1;
}

static int
setGamestateEventHandlers(){
  int mt;

  for (mt=PROTO_MT_EVENT_BASE_RESERVED_FIRST+1;
       mt<PROTO_MT_EVENT_BASE_RESERVED_LAST; mt++){
    if(mt == PROTO_MT_EVENT_BASE_MOVE)
      proto_client_set_event_handler(Client.ph, mt, event_move_handler);
    else if(mt == PROTO_MT_EVENT_BASE_PICKUP)
      proto_client_set_event_handler(Client.ph, mt, event_pickup_handler);
    else if(mt == PROTO_MT_EVENT_BASE_DROP)
      proto_client_set_event_handler(Client.ph, mt, event_drop_handler);
    else if(mt == PROTO_MT_EVENT_BASE_WIN)
      proto_client_set_event_handler(Client.ph, mt, event_win_handler);
    else if(mt == PROTO_MT_EVENT_BASE_PLAYER_QUIT)
      proto_client_set_event_handler(Client.ph, mt, event_player_quit_handler);
    else if(mt == PROTO_MT_EVENT_BASE_PLAYER_JOIN)
      proto_client_set_event_handler(Client.ph, mt, event_player_join_handler);
  }
  return 1;
}


int 
doRPC(char c) 
{
  Proto_Msg_Hdr hdr;
  int rc=-1;
  int x,y;
  int offset=0;

  Proto_Session *s;
  switch (c) {
  case 'h':
    rc = proto_client_hello(Client.ph);
    s = proto_client_rpc_session(Client.ph);
    proto_session_hdr_unmarshall(s,&hdr);
    if(s->rhdr.pstate.v0.raw){
      globals.x = (unsigned char)s->rhdr.pstate.v1.raw;
      globals.y = (unsigned char)s->rhdr.pstate.v2.raw;
      offset = proto_session_body_unmarshall_gamestate(s,0,Client.Gamestate);
      gamestate_dump(Client.Gamestate);
      proto_session_body_unmarshall_map(s,offset,Client.Map);
      setGamestateEventHandlers();
    }
    break;
  case 'i':
    rc = proto_client_cinfo(Client.ph, globals.x, globals.y);
    s = proto_client_rpc_session(Client.ph);
    proto_session_body_unmarshall_cell(s, 0, globals.cell);
    break;
  case 'v':
    //currently no validity checking client-side
    rc = proto_client_move(Client.ph, Client.Player->id, globals.mv);
    s = proto_client_rpc_session(Client.ph);
    if(s->rhdr.pstate.v3.raw > 0){
      if(s->rhdr.pstate.v0.raw == Client.Player->id){
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
    rc = proto_client_dump(Client.ph);
    break;
  case 'g':
    rc = proto_client_goodbye(Client.ph,Client.Player);
    break;
  default:
    printf("%s: unknown command %c\n", __func__, c);
  }
  if (rc == 0xdeadbeef) rc=1;
  return rc;
}

int
doMove(){
  int rc,x;
  char ch;

  if((ch=getchar())=='\n'){
    printf("Failed to move, usage move <1/2/3/4>\n");
    return 1;
  }

  putchar(ch);
  scanf("%c", &ch);

  if(!Client.connected){
    printf("Not Connected.");
    return 1;
  }

  x = atoi(&ch)-1;

  if(x < 0 || x > 3){
    printf("Failed to move, usage move <1/2/3/4>\n");
    return 1;
  }

  globals.mv = x;
  
  rc = doRPC('v');

  return rc;
}

extern int
doDisconnect(char cmd){
  Proto_Session *rpc;
  Proto_Session *event;
  
  rpc = proto_client_rpc_session(Client.ph);
  event = proto_client_event_session(Client.ph);

  if(cmd == 'q' && !Client.connected){
    printf("Terminated.\n");
    return -1;
  }

  if(!Client.connected){
    printf("Not Connected.");
    return 1;
  }

  if(cmd != 's'){
    doRPC('g'); //goodbye
  }

  net_close_socket(rpc->fd);
  net_close_socket(event->fd);

  Client.connected = 0;

  if(cmd=='q'){
    printf("Terminated.\n");
    return -1;
  }
  if(cmd == 's'){
    fprintf(stderr, "Disconnected: Server Quit.\n");
    return 1;
  }
  printf("Disconnected");
  return 1;
}

static int 
event_server_quit_handler(Proto_Session *s){
  return doDisconnect('s');
}

int
doConnect(){
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

  if(Client.connected){
    printf("Already Connected.");
    return 1;
  }  

  *ptr = '\0';
  
  strncpy(globals.host, addr_port, strlen(addr_port));
  globals.port = atoi(ptr+sizeof(char));
  
  if (startConnection(globals.host, globals.port, update_event_handler)<0) {
    fprintf(stderr, "ERROR: startConnection failed\n");
    return -1;
  }
  else{
    doRPC('h');
    printf("Connected.");
    printf("\n");
    player_dump(Client.Player);
    Client.connected = 1;
  }
  return 1;
}

int
doWhere(){
  if(Client.connected)
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
    if(!Client.connected){
      printf("Not Connected.");
      return 1;
    }
    printf("Number of home cells for team %d = ", teamNum);
    if(teamNum == 1)
      printf("%d\n",Client.Map->numhome1);
    else
      printf("%d\n",Client.Map->numhome2);
  } else {
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
    if(!Client.connected){
      printf("Not Connected.");
      return 1;
    }
    printf("Number of jail cells for team %d = ", teamNum);
    if(teamNum == 1)
      printf("%d\n",Client.Map->numjail1);
    else
      printf("%d\n",Client.Map->numjail2);
  }
  else{
    printf("Usage: \"numjail <1 or 2>\"\n");
    return 1;
  }

  return 1;
}

int
doNumFloor(){
  if(!Client.connected){
    printf("Not Connected.");
    return 1;
  }
  printf("Number of available floor cells = %d\n", Client.Map->numfloor);
  return 1;
}

int
doNumWall(){
  if(!Client.connected){
    printf("Not connected.");
    return 1;
  }    
  printf("Number of available wall cells = %d\n", Client.Map->numwall);
  return 1;
}

int
doDim(){
  if(!Client.connected){
    printf("Not Connected.");
    return 1;
  }    
  printf("Dimensions of the maze are %d x %d\n", Client.Map->dim-1,Client.Map->dim-1);
  return 1;
}

int 
doCInfo(){
  char c;
  int rc,x,y;

  if((c=getchar())=='\n'){
    printf("Usage: \"cinfo <x,y>\"\n");
    return 1;
  }
  putchar(c);
  
  if(scanf("%d,%d", &x, &y)==2){
    if(!Client.connected){
      printf("Not Connected.");
      return 1;
    }        
    
    globals.x = x;
    globals.y = y;
    rc = doRPC('i');
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
doDump(){
  int rc;

  if(!Client.connected){
    printf("Not Connected.");
    return 1;
  }
  
  if((rc=doRPC('d'))>0){
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
docmd(char cmd)
{
  int rc = 1;

  switch (cmd) {
  case 'c': 
    rc=doConnect();
    break;
  case 'd': 
    rc=doDisconnect(cmd);
    break;
  case 'w': 
    rc=doWhere(); 
    break;
  case 'q':
    rc=doDisconnect(cmd);
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
    rc = doCInfo();
    break;
  case 'u': 
    rc = doDump();
    break;
  case 'v': 
    rc = doMove();
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

static int
clientInit(){
  int mt;

  Client.Player = (Player *)malloc(sizeof(Player));
  Client.Map = (Map *)malloc(sizeof(Map));
  globals.cell = (Cell *)malloc(sizeof(Cell));

  bzero(Client.Player,sizeof(Client.Player));
  bzero(Client.Map,sizeof(Client.Map));
  bzero(globals.cell,sizeof(globals.cell));
  
  Client.Gamestate = gamestate_create();
  
  if (proto_client_init(&(Client.ph))<0) {
    fprintf(stderr, "client: main: ERROR initializing proto system\n");
    return -1;
  }
  
  proto_client_set_session_lost_handler(Client.ph, proto_client_session_lost_default_hdlr);
  
  for (mt=PROTO_MT_EVENT_BASE_RESERVED_FIRST+1;
       mt<PROTO_MT_EVENT_BASE_RESERVED_LAST; mt++){
    if(mt == PROTO_MT_EVENT_BASE_SERVER_QUIT)
      proto_client_set_event_handler(Client.ph, mt, event_server_quit_handler);
    else
      proto_client_set_event_handler(Client.ph, mt, event_null_handler);
  }
  
  return 1;
}

void *
shell(void *arg)
{
  char c;
  int rc;
  int menu=1;

  while (1) {
    if ((c=prompt(menu))!=0) rc=docmd(c);
    if (rc<0) break;
    if (rc==1) menu=1; else menu=0;
  }

  fflush(stdout);
  return NULL;
}

int 
main(int argc, char **argv)
{
  bzero(&globals, sizeof(globals));
  
  if (clientInit() < 0) {
    fprintf(stderr, "ERROR: clientInit failed\n");
    return -1;
  }    
  
  shell(NULL);
  
  return 0;
}
