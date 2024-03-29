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
#include <strings.h>
#include <errno.h>
#include <pthread.h>

#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_client.h"
#include "map.h"

typedef struct {
  int connected;
  Gamestate *Client_Gamestate;
  Proto_Session rpc_session;
  Proto_Session event_session;
  pthread_t EventHandlerTid;
  Proto_MT_Handler session_lost_handler;
  Proto_MT_Handler base_event_handlers[PROTO_MT_EVENT_BASE_RESERVED_LAST 
				       - PROTO_MT_EVENT_BASE_RESERVED_FIRST
				       - 1];
} Proto_Client;

extern Proto_Session *
proto_client_rpc_session(Proto_Client_Handle ch){
  Proto_Client *c = ch;
  return &(c->rpc_session);
}

extern Proto_Session *
proto_client_event_session(Proto_Client_Handle ch){
  Proto_Client *c = ch;
  return &(c->event_session);
}

extern int
proto_client_set_session_lost_handler(Proto_Client_Handle ch, Proto_MT_Handler h){
  Proto_Client *c = ch;
  c->session_lost_handler = h;
}

extern int
proto_client_set_event_handler(Proto_Client_Handle ch, Proto_Msg_Types mt,
			       Proto_MT_Handler h){
  int i;
  Proto_Client *c = ch;
  
  if (mt>PROTO_MT_EVENT_BASE_RESERVED_FIRST && 
      mt<PROTO_MT_EVENT_BASE_RESERVED_LAST) {
    
    i=mt - PROTO_MT_EVENT_BASE_RESERVED_FIRST - 1;
    
    c->base_event_handlers[i] = h;
    
    return 1;
  } else {
    return -1;
  }
}

static void *
proto_client_event_dispatcher(void * arg){
  Proto_Client *c;
  Proto_Session *s;
  Proto_Msg_Types mt;
  Proto_MT_Handler hdlr;
  int i;

  pthread_detach(pthread_self());

  c = (Proto_Client *)arg;
  s = &(c->event_session);

  for (;;) {
    if (proto_session_rcv_msg(s)==1) {
      mt = proto_session_hdr_unmarshall_type(s);
      if (mt > PROTO_MT_EVENT_BASE_RESERVED_FIRST && 
	  mt < PROTO_MT_EVENT_BASE_RESERVED_LAST) {

	i = mt - PROTO_MT_EVENT_BASE_RESERVED_FIRST - 1;	
	hdlr = c->base_event_handlers[i];

	if (hdlr(s)<0) goto leave;
      }
    } else {
      c->session_lost_handler;
      goto leave;
    }
  }
 leave:
  close(s->fd);
  return NULL;
}

extern int
proto_client_init(Proto_Client_Handle *ch){
  Proto_Msg_Types mt;
  Proto_Client *c;
 
  c = (Proto_Client *)malloc(sizeof(Proto_Client));
  if (c==NULL) return -1;
  bzero(c, sizeof(Proto_Client));
  
  *ch = c;
  return 1;
}

int
proto_client_connect(Proto_Client_Handle ch, char *host, PortType port){
  Proto_Client *c = (Proto_Client *)ch;

  if (net_setup_connection(&(c->rpc_session.fd), host, port)<0) 
    return -1;

  if (net_setup_connection(&(c->event_session.fd), host, port+1)<0) 
    return -2; 

  if (pthread_create(&(c->EventHandlerTid), NULL, 
		     &proto_client_event_dispatcher, c) !=0) {
    fprintf(stderr, 
	    "proto_client_init: create EventHandler thread failed\n");
    perror("proto_client_init:");
    return -3;
  }

  return 0;
}

static void
marshall_mtonly(Proto_Session *s, Proto_Msg_Types mt) {
  Proto_Msg_Hdr h;
  
  bzero(&h, sizeof(h));
  h.type = mt;
  proto_session_hdr_marshall(s, &h);
}

static int
do_generic_dummy_rpc(Proto_Client_Handle ch, Proto_Msg_Types mt){
  int rc;
  Proto_Session *s;
  Proto_Client *c = ch;

  s = &(c->rpc_session);  

  marshall_mtonly(s, mt);

  rc = proto_session_rpc(s);

  if (rc==1) {
    if(mt == PROTO_MT_REQ_BASE_HELLO){
      proto_session_hdr_unmarshall(s,&s->rhdr);
    } else
      proto_session_body_unmarshall_int(s, 0, &rc);
  } else {
    c->session_lost_handler(s);
  }

  return rc;
}

static int
do_goodbye_rpc(Proto_Client_Handle ch, Player *p, Proto_Msg_Types mt){
  int rc=0;
  Proto_Session *s;
  Proto_Client *c = ch;
  Proto_Msg_Hdr hdr;

  s = &(c->rpc_session);  

  bzero(&hdr, sizeof(Proto_Msg_Hdr));

  hdr.type = mt;
  proto_session_hdr_marshall(s,&hdr);
  proto_session_body_marshall_player(s,p);

  rc = proto_session_rpc(s);

  if(rc==1){
    proto_session_hdr_unmarshall(s,&s->rhdr);
    //    proto_dump_msghdr(&(s->rhdr));
  }
  else 
    c->session_lost_handler(s);
  return rc;
}

static int
do_move_rpc(Proto_Client_Handle ch, int playerid, int dir, Proto_Msg_Types mt){
  int rc=0;
  Proto_Session *s;
  Proto_Client *c = ch;
  Proto_Msg_Hdr *hdr = malloc(sizeof(Proto_Msg_Hdr));

  s = &(c->rpc_session);  

  bzero(hdr, sizeof(Proto_Msg_Hdr));

  hdr->pstate.v0.raw=playerid;
  hdr->pstate.v1.raw=dir;
  hdr->type = mt;

  proto_session_hdr_marshall(s,hdr);

  rc = proto_session_rpc(s);

  if(rc==1){
    proto_session_hdr_unmarshall(s,&s->rhdr);
    //    proto_dump_msghdr(&(s->rhdr));
  }
  else 
    c->session_lost_handler(s);
  return rc;
}

static int
do_cinfo_rpc(Proto_Client_Handle ch, int x, int y, Proto_Msg_Types mt){
  int rc=0;
  Proto_Session *s;
  Proto_Client *c = ch;
  Proto_Msg_Hdr *hdr = malloc(sizeof(Proto_Msg_Hdr));

  s = &(c->rpc_session);  

  bzero(hdr, sizeof(Proto_Msg_Hdr));

  hdr->pstate.v0.raw=x;
  hdr->pstate.v1.raw=y;
  hdr->type = mt;

  proto_session_hdr_marshall(s,hdr);

  proto_dump_msghdr(&(s->shdr));

  rc = proto_session_rpc(s);
  
  if(rc==1){
    proto_session_hdr_unmarshall(s,&s->rhdr);
    proto_dump_msghdr(&(s->rhdr));
  }
  else 
    c->session_lost_handler(s);

  return rc;
}

extern int
proto_client_hello(Proto_Client_Handle ch){
  return do_generic_dummy_rpc(ch,PROTO_MT_REQ_BASE_HELLO);
}

extern int
proto_client_move(Proto_Client_Handle ch, int playerid, int dir){
  return do_move_rpc(ch, playerid, dir, PROTO_MT_REQ_BASE_MOVE);
}

extern int
proto_client_dump(Proto_Client_Handle ch){
  int rc;

  rc = do_generic_dummy_rpc(ch,PROTO_MT_REQ_BASE_DUMP);

  return rc;
}

extern int
proto_client_cinfo(Proto_Client_Handle ch, int x, int y){
  int rc=0;
  rc = do_cinfo_rpc(ch, x, y, PROTO_MT_REQ_BASE_CINFO);
  return rc;
}

extern int
proto_client_goodbye(Proto_Client_Handle ch, Player *p){
  int rc;
  Proto_Session *s;
  Proto_Client *c = ch;

  do_goodbye_rpc(ch,p,PROTO_MT_REQ_BASE_GOODBYE);
  /* 
     s = &(c->rpc_session);
     proto_client_session_lost_default_hdlr(s);
     s = &(c->event_session);
     proto_client_session_lost_default_hdlr(s);
  */

  return 1;
}
