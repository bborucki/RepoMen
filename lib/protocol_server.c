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
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "map.h"
#include "cell.h"
#include "net.h"
#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_server.h"
#include "protocol_session.h"
#include "objectmap.h"
#include "player.h"
#include "gamestate.h"

#define DEBUG_MAP 0

#define PROTO_SERVER_MAX_EVENT_SUBSCRIBERS 1024
#define MAX_OBJECTS 404

Gamestate *Server_Gamestate;
Map *Server_Map;
ObjectMap *Server_ObjectMap; 
int pidx;
team_t nextTeam;

struct {
  FDType   RPCListenFD;
  PortType RPCPort;

  FDType                    EventListenFD;
  PortType                  EventPort;
  pthread_t                 EventListenTid;
  pthread_mutex_t           EventSubscribersLock;
  int                       EventLastSubscriber;
  int                       EventNumSubscribers;
  FDType                    EventSubscribers[PROTO_SERVER_MAX_EVENT_SUBSCRIBERS];
  Proto_Session             EventSession; 
  pthread_t                 RPCListenTid;
  Proto_MT_Handler   session_lost_handler;
  Proto_MT_Handler   base_req_handlers[PROTO_MT_REQ_BASE_RESERVED_LAST - 
				       PROTO_MT_REQ_BASE_RESERVED_FIRST-1];
} Proto_Server;


extern PortType proto_server_rpcport(void) { return Proto_Server.RPCPort; }
extern PortType proto_server_eventport(void) { return Proto_Server.EventPort; }
extern Proto_Session *
proto_server_event_session(void) 
{ 
  return &Proto_Server.EventSession; 
}

extern ObjectMap *
proto_server_objectmap(void) 
{ 
  return Server_ObjectMap; 
}

extern int
proto_server_set_session_lost_handler(Proto_MT_Handler h){
  Proto_Server.session_lost_handler = h;
}

extern int
proto_server_set_req_handler(Proto_Msg_Types mt, Proto_MT_Handler h){
  int i;
  
  if (mt>PROTO_MT_REQ_BASE_RESERVED_FIRST &&
      mt<PROTO_MT_REQ_BASE_RESERVED_LAST) {
    i = mt - PROTO_MT_REQ_BASE_RESERVED_FIRST - 1;
    
    Proto_Server.base_req_handlers[i] = h;
    
    return 1;
  } else {
    return -1;
  }
}

static int
proto_server_record_event_subscriber(int fd, int *num){
  int rc=-1;

  pthread_mutex_lock(&Proto_Server.EventSubscribersLock);

  if (Proto_Server.EventLastSubscriber < PROTO_SERVER_MAX_EVENT_SUBSCRIBERS
      && Proto_Server.EventSubscribers[Proto_Server.EventLastSubscriber]
      ==-1) {
    Proto_Server.EventNumSubscribers++;
    *num = Proto_Server.EventLastSubscriber++;
    Proto_Server.EventSubscribers[*num]=fd;
    rc = 1;
  } else { //else search for first free array element
    int i;
    for (i=0; i< PROTO_SERVER_MAX_EVENT_SUBSCRIBERS; i++) {
      if (Proto_Server.EventSubscribers[i]==-1) {
	Proto_Server.EventSubscribers[i]=fd;
	Proto_Server.EventNumSubscribers++;
	//	NYI;

	*num=i;
	rc=1;
	break;
      }
    }
  }

  pthread_mutex_unlock(&Proto_Server.EventSubscribersLock);

  return rc;
}

static void *
proto_server_event_listen(void *arg){
  int fd = Proto_Server.EventListenFD;
  int connfd;

  if (net_listen(fd)<0) {
    exit(-1);
  }

  for (;;) {
    connfd = net_accept(fd);
    if (connfd < 0) {
      fprintf(stderr, "Error: EventListen accept failed (%d)\n", errno);
   } else {
      int i;
      fprintf(stderr, "EventListen: connfd=%d -> ", connfd);
      if (proto_server_record_event_subscriber(connfd, &i)<0){
	fprintf(stderr, "oops no space for any more event subscribers\n");
	close(connfd);
      } else {
	fprintf(stderr, "subscriber num %d\n", i);
      }
    } 
  }
} 

void
proto_server_post_event(void){
  int i;
  int num;

  pthread_mutex_lock(&Proto_Server.EventSubscribersLock);

  i = 0;
  num = Proto_Server.EventNumSubscribers;

  while (num) {
    Proto_Server.EventSession.fd = Proto_Server.EventSubscribers[i];
    if (Proto_Server.EventSession.fd != -1) {
      num--;
      
      if (proto_session_send_msg(&(Proto_Server.EventSession),0)<0) {

	printf("Lost a connection.\n");

	// must have lost an event connection
	close(Proto_Server.EventSession.fd);
	Proto_Server.EventSubscribers[i]=-1;
	Proto_Server.EventNumSubscribers--;
	//	NYI; //Proto_Server.ADD CODE
	//I Have no idea^^ -B 4/24/13
	// may have to issue a remove player for this subscriber...-chris 4/24/13
      } 
      // FIXME: add ack message here to ensure that game is updated 
      // correctly everywhere... at the risk of making server dependent
      // on client behaviour  (use time out to limit impact... drop
      // clients that misbehave but be carefull of introducing deadlocks
    }
    i++;  }
  proto_session_reset_send(&Proto_Server.EventSession);
  pthread_mutex_unlock(&Proto_Server.EventSubscribersLock);
}


static void *
proto_server_req_dispatcher(void * arg){
  Proto_Session s;
  Proto_Msg_Types mt;
  Proto_MT_Handler hdlr;
  int i;
  int ret;
  unsigned long arg_value = (unsigned long) arg;
  
  pthread_detach(pthread_self());

  proto_session_init(&s);

  s.fd = (FDType) arg_value;

  fprintf(stderr, "\nproto_req_dispatcher: %p: Started: fd=%d\n", 
	  pthread_self(), s.fd);

  for (;;){
    if (proto_session_rcv_msg(&s)==1) {
      mt = proto_session_hdr_unmarshall_type(&s);
      i = mt - PROTO_MT_REQ_BASE_RESERVED_FIRST - 1;
      hdlr = Proto_Server.base_req_handlers[i];

      ret = hdlr(&s);

      if (ret<0) goto leave;
    }
    else {
      goto leave;
    }
  }

 leave:
  Proto_Server.session_lost_handler;

  close(s.fd);
  return NULL;
}

static void *
proto_server_rpc_listen(void *arg){
  int fd = Proto_Server.RPCListenFD;
  unsigned long connfd;
  pthread_t tid;
  
  if (net_listen(fd) < 0) {
    fprintf(stderr, "Error: proto_server_rpc_listen listen failed (%d)\n", errno);
    exit(-1);
  }

  for (;;) {
    connfd = net_accept(fd); //not accepting new rpc requests
    if (connfd < 0) {
      fprintf(stderr, "Error: proto_server_rpc_listen accept failed (%d)\n", errno);
    } else {
      pthread_create(&tid, NULL, &proto_server_req_dispatcher,
		     (void *)connfd);
    }
  }
}

extern int
proto_server_start_rpc_loop(void){
  if (pthread_create(&(Proto_Server.RPCListenTid), NULL, 
		     &proto_server_rpc_listen, NULL) !=0) {
    fprintf(stderr, 
	    "proto_server_rpc_listen: pthread_create: create RPCListen thread failed\n");
    perror("pthread_create:");
    return -3;
  }
  return 1;
}

static int 
proto_session_lost_default_handler(Proto_Session *s){
  fprintf(stderr, "Player %d has disconnected\n", s->player->id);
  Player *p;
  p = s->player;
  if(Server_Gamestate->plist[s->player->id] != NULL){
    gamestate_remove_player(Server_Gamestate, s->player->id);
    objectmap_remove_player(s->player->x,s->player->y, Server_ObjectMap);
  }
  if(p->team == TEAM1){
    numPlayers1--;
    if(p->state == SAFE)
      numplayershome1--;
  } else{
    numPlayers2--;
    if(p->state == SAFE)
      numplayershome2--;
  }
  pidx = player_find_next_id(Server_Gamestate->plist);
  
  //  proto_session_dump(s);
  return -1;
}

static int 
proto_server_mt_null_handler(Proto_Session *s){
  int rc=1;
  Proto_Msg_Hdr h;
  
  fprintf(stderr, "proto_server_mt_null_handler: invoked for session:\n");
  proto_session_dump(s);

  // setup dummy reply header : set correct reply message type and 
  // everything else empty
  bzero(&h, sizeof(h));
  h.type = proto_session_hdr_unmarshall_type(s);
  h.type += PROTO_MT_REP_BASE_RESERVED_FIRST;
  proto_session_hdr_marshall(s, &h);

  // setup a dummy body that just has a return code 
  proto_session_body_marshall_int(s, 0xdeadbeef);

  rc=proto_session_send_msg(s,1);

  return rc;
}


static int
proto_server_hello_handler(Proto_Session *s){
  int rc = 1;
  int x;
  Player* p;
  Proto_Msg_Hdr sh;
  Proto_Session *us;

  p = player_create();

  bzero(&sh, sizeof(sh));
  
  sh.type = proto_session_hdr_unmarshall_type(s);
  sh.type += PROTO_MT_REP_BASE_RESERVED_FIRST;

  x = player_find_empty_home(p,nextTeam, Server_ObjectMap, pidx);
  //  x = !gamefull;
  if(x){
    gamestate_add_player(Server_Gamestate,p);
    printf("New player joining:\n");
    printf("Location: %d,%d\n", p->pcell->x, p->pcell->y);
    player_dump(p);
    sh.pstate.v0.raw = 1;
    sh.pstate.v1.raw = p->id;
    gamestate_dump(Server_Gamestate);
    proto_session_body_marshall_gamestate(s,Server_Gamestate);
    //    proto_session_body_marshall_map(s,Server_Map);
    if(nextTeam == TEAM1){
      nextTeam = TEAM2;
      numPlayers1++;
    } else{
      nextTeam = TEAM1;
      numPlayers2++;
    }
    pidx = player_find_next_id(Server_Gamestate->plist);
    if(pidx<0)
      gamefull = 1;

    s->player = p;
  } else {
    sh.pstate.v0.raw = 0;
  }

  proto_session_hdr_marshall(s, &sh);
  
  //  proto_dump_msghdr(&(s->shdr));
  rc = proto_session_send_msg(s,1);
  
  if(x){ //if new player joining, tell everyone    
    us = proto_server_event_session();
    bzero(&sh, sizeof(sh));
    sh.type = PROTO_MT_EVENT_BASE_PLAYER_JOIN;
    sh.pstate.v0.raw = p->id;
    sh.pstate.v1.raw = p->pcell->x;
    sh.pstate.v2.raw = p->pcell->y;
    proto_session_hdr_marshall(us, &sh);
    proto_session_body_marshall_player(us,p);
    proto_server_post_event();
  }

  return rc;
}
/*
  static int
  proto_server_query_handler(Proto_Session *s){
  int rc = 1;
  Proto_Msg_Hdr sh;
  bzero(&sh, sizeof(sh));
  
  fprintf(stderr, "proto_server_mt_query_handler: invoked for session:\n");
  proto_session_dump(s);
  
  sh.type = proto_session_hdr_unmarshall_type(s);
  sh.type += PROTO_MT_REP_BASE_RESERVED_FIRST;
  
  proto_session_hdr_marshall(s, &sh);
  proto_session_body_marshall_map(s,Server_Map);
  
  rc = proto_session_send_msg(s,1);
  
  return rc;
  }
*/

static int
proto_server_cinfo_handler(Proto_Session *s){

  fprintf(stderr, "proto_server_mt_cinfo_handler: invoked for session:\n");

  /*  int rc = 1;
  int i,rx,ry;
  Cell *cell = malloc(sizeof(Cell));
  Proto_Msg_Hdr sh;
  Proto_Msg_Hdr rh;

  bzero(&sh, sizeof(sh));
  bzero(&rh, sizeof(rh));

  fprintf(stderr, "proto_server_cinfo_handler: invoked for session:\n");
  proto_session_dump(s);

  sh.type = proto_session_hdr_unmarshall_type(s);
  sh.type += PROTO_MT_REP_BASE_RESERVED_FIRST;

  proto_session_hdr_unmarshall(s, &rh);
  rx = rh.pstate.v0.raw;
  ry = rh.pstate.v1.raw; 

  for(i=0; i<MAX_OBJECTS; i+=sizeof(object)){
    if(objects[i] != NULL){
      if(objects[i]->x == rx && objects[i]->y == ry){
	cell = objects[i];
	break;
      }
    }
  }

  if(i >= MAX_OBJECTS){
    cell_create(Server_Map,cell,rx,ry);
  }

  proto_session_body_marshall_cell(s, cell);

  proto_session_hdr_marshall(s, &sh);

  printf("sending cinfo\n");
  
  rc = proto_session_send_msg(s,1);

  printf("sent cinfo\n");
  
  return rc;*/
}

static int
proto_server_pickup_handler(Proto_Session *s){
  int id,rc,ret;
  Proto_Msg_Hdr sh;
  Proto_Msg_Hdr rh;
  Player* p;
  Proto_Session *us;

  bzero(&sh, sizeof(sh));
  bzero(&rh, sizeof(rh));

  sh.type = proto_session_hdr_unmarshall_type(s);
  sh.type += PROTO_MT_REP_BASE_RESERVED_FIRST;

  proto_session_hdr_unmarshall(s, &rh);
  id = rh.pstate.v0.raw;
  p = gamestate_get_player(Server_Gamestate,id);
  
  ret = player_obj_pickup(p,Server_ObjectMap,Server_Gamestate);
  sh.pstate.v0.raw = id;//same header for both reply and update
  sh.pstate.v1.raw = ret; //sending back the actual thing picked up
  proto_session_hdr_marshall(s, &sh);
  rc = proto_session_send_msg(s,1);
  
  if(ret >= 0){
    us = proto_server_event_session();
    sh.type = PROTO_MT_EVENT_BASE_MOVE;
    proto_session_hdr_marshall(us,&sh);
    proto_server_post_event();
  } 
  return rc;
}

static int
proto_server_move_handler(Proto_Session *s){
  int i,rx,ry,id,rc, winner;
  dir_t dir;
  Cell *cell = malloc(sizeof(Cell));
  Proto_Msg_Hdr sh;
  Proto_Msg_Hdr rh;
  Player* p;
  Proto_Session *us;
  int valid;
  int flagindex;
  object_t flag = -1;
  Proto_Session *fs;
  bzero(&sh, sizeof(sh));
  bzero(&rh, sizeof(rh));

  sh.type = proto_session_hdr_unmarshall_type(s);
  sh.type += PROTO_MT_REP_BASE_RESERVED_FIRST;

  proto_session_hdr_unmarshall(s, &rh);
  id = rh.pstate.v0.raw;
  dir = rh.pstate.v1.raw;
  p = gamestate_get_player(Server_Gamestate,id);
  valid = 0;
  flagindex = -1;
  valid = player_move(dir,p,Server_ObjectMap, Server_Gamestate);
  //  printf("Valid bit%d\n", valid);
  if (valid>0) {
    sh.pstate.v3.raw = 1;
    printf("Player %d is moving to (%d,%d)\n",id,p->pcell->x,p->pcell->y);
    if(!DEBUG_MAP){
      flagindex = objectmap_flag_visible(p,Server_ObjectMap);
      
      if(flagindex>=0)
	flag = Server_ObjectMap->objects[flagindex]->obj;
      
      if(flag == FLAG1){
	if(!flag1found)
	  flag1found = 1;
	else
	  flagindex = -1;
      }
      if(flag == FLAG2){
	if(!flag2found)
	  flag2found = 1;
	else
	  flagindex = -1;
      }
    }
  } else {
    sh.pstate.v3.raw = 0;    
    printf("Player %d attemped an invalid move\n",id);
  }
  sh.pstate.v0.raw = p->id;
  sh.pstate.v1.raw = p->pcell->x;
  sh.pstate.v2.raw = p->pcell->y;
 
  proto_session_hdr_marshall(s, &sh);
  
  rc = proto_session_send_msg(s,1);
  
  if(valid){
    us = proto_server_event_session();
    sh.type = PROTO_MT_EVENT_BASE_MOVE;
    proto_session_hdr_marshall(us,&sh);
    proto_server_post_event();
  }
  
  bzero(&sh, sizeof(sh));
  if(flagindex>=0){
    fs = proto_server_event_session();
    sh.type = PROTO_MT_EVENT_BASE_FLAG;
    
    //gstate.v0 holds the flag itself
    //gstate.v1 holds the index in the objectmap where that flag is located
    sh.gstate.v0.raw = Server_ObjectMap->objects[flagindex]->obj;
    sh.gstate.v1.raw = flagindex;
    proto_session_hdr_marshall(fs,&sh);
    proto_server_post_event();
  }
  
  bzero(&sh, sizeof(sh));
  if((winner = gamestate_team_wins()) >= 0){
    sh.type = PROTO_MT_EVENT_BASE_WIN;
    sh.gstate.v0.raw = winner;
    proto_session_hdr_marshall(fs,&sh);
    proto_server_post_event();
  }

  return rc;
}

static int
proto_server_dump_handler(Proto_Session *s){
  Proto_Msg_Hdr h;
  int i;
  
  map_dump(Server_Map);

  bzero(&h, sizeof(h));
  h.type = PROTO_MT_REP_BASE_DUMP;
  proto_session_hdr_marshall(s, &h);
  proto_session_body_marshall_int(s, 1);

  return proto_session_send_msg(s,1);
}

//send back an ack reply then close session
static int
proto_server_goodbye_handler(Proto_Session *s){
  Proto_Msg_Hdr h;
  Proto_Session *us;
  Player p;
  int rc=1;
  
  proto_session_body_unmarshall_player(s,0,&p);

  bzero(&h, sizeof(h));
  h.type = PROTO_MT_REP_BASE_GOODBYE;
  proto_session_hdr_marshall(s, &h);

  rc = proto_session_send_msg(s,1);

  if(p.team == TEAM1){
    numPlayers1--;
    if(p.state == SAFE)
      numplayershome1--;
  } else{
    numPlayers2--;
    if(p.state == SAFE)
      numplayershome2--;
  }
  gamestate_remove_player(Server_Gamestate,p.id);
  pidx = player_find_next_id(Server_Gamestate->plist);
  if(pidx<0)
    gamefull = 1;
  else
    gamefull = 0;

  us = proto_server_event_session();
  h.type = PROTO_MT_EVENT_BASE_PLAYER_QUIT;
  proto_session_hdr_marshall(us,&h);
  proto_session_body_marshall_player(us, &p);
  proto_server_post_event();

  return rc;
}

extern int
proto_server_init(void){
  int i;
  int rc;
  int dim;

  gamefull=flag1found=flag2found=flag1home1=flag2home1=flag1home2=
    flag2home2=numplayershome1=numplayershome2=numPlayers1=numPlayers2 = 0;
  
  if((Server_Map = map_init(MAP_NAME)) == NULL)
    return -1;
  dim = Server_Map->dim;

  if((Server_Gamestate = gamestate_create()) == NULL){
    fprintf(stderr, "could not create gamestate\n");
    return -1;
  }

  if((Server_ObjectMap = objectmap_create(Server_Map, Server_Gamestate)) == NULL){
    fprintf(stderr, "could not load map\n");
    return -1;
  }

  pidx = 0;
  nextTeam = TEAM1;

  proto_session_init(&Proto_Server.EventSession);

  proto_server_set_session_lost_handler(proto_session_lost_default_handler);

  for (i=PROTO_MT_REQ_BASE_RESERVED_FIRST+1; 
       i<PROTO_MT_REQ_BASE_RESERVED_LAST; i++) {
    if(i == PROTO_MT_REQ_BASE_HELLO)
      proto_server_set_req_handler(i,proto_server_hello_handler);
    else if(i == PROTO_MT_REQ_BASE_MOVE)
      proto_server_set_req_handler(i,proto_server_move_handler);
    //    if(i == PROTO_MT_REQ_BASE_QUERY)
    //      proto_server_set_req_handler(i,proto_server_query_handler);
    else if(i == PROTO_MT_REQ_BASE_DUMP)
      proto_server_set_req_handler(i,proto_server_dump_handler);
    else if(i == PROTO_MT_REQ_BASE_CINFO)
      proto_server_set_req_handler(i,proto_server_cinfo_handler);
    else if(i == PROTO_MT_REQ_BASE_GOODBYE)
      proto_server_set_req_handler(i,proto_server_goodbye_handler);
    else
      proto_server_set_req_handler(i, proto_server_mt_null_handler);
  }
  for (i=0; i<PROTO_SERVER_MAX_EVENT_SUBSCRIBERS; i++) {
    Proto_Server.EventSubscribers[i]=-1;
  }
  Proto_Server.EventNumSubscribers=0;
  Proto_Server.EventLastSubscriber=0;
  pthread_mutex_init(&Proto_Server.EventSubscribersLock, 0);

  rc=net_setup_listen_socket(&(Proto_Server.RPCListenFD),
			     &(Proto_Server.RPCPort));

  if (rc==0) { 
    fprintf(stderr, "prot_server_init: net_setup_listen_socket: FAILED for RPCPort\n");
    return -1;
  }

  Proto_Server.EventPort = Proto_Server.RPCPort + 1;

  rc=net_setup_listen_socket(&(Proto_Server.EventListenFD),
			     &(Proto_Server.EventPort));

  if (rc==0) { 
    fprintf(stderr, "proto_server_init: net_setup_listen_socket: FAILED for EventPort=%d\n", 
	    Proto_Server.EventPort);
    return -2;
  }

  if (pthread_create(&(Proto_Server.EventListenTid), NULL, 
		     &proto_server_event_listen, NULL) !=0) {
    fprintf(stderr, 
	    "proto_server_init: pthread_create: create EventListen thread failed\n");
    perror("pthread_createt:");
    return -3;
  }

  return 0;
}
