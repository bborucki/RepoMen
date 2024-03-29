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

#include "protocol.h"
#include "protocol_utils.h"
#include "types.h"

int PROTO_DEBUG=0;

extern void
dump_team(team_t team){
  switch (team) {
  case TEAM1:
    printf("Team 1\n");
    break;
  case TEAM2:
    printf("Team 2\n");
    break;
  default:
    printf("UNKNOWN=%d", team);
  }
}

extern void
dump_state(state_t state){
  switch (state) {
  case SAFE:
    printf("State: safe\n");
    break;
  case JAILED:
    printf("State: jailed\n");
    break;
  case FREE:
    printf("State: free\n");
    break;
  default:
    printf("UNKNOWN=%d", state);
  }
}

extern void
dump_object(object_t object){
  switch (object) {
  case NONE:
    printf("No object\n");
    break;
  case FLAG1:
    printf("Flag 1\n");
    break;
  case FLAG2:
    printf("Flag 2\n");
    break;
  case SHOVEL1:
    printf("Shovel 1\n");
    break;
  case SHOVEL2:
    printf("Shovel 2\n");
    break;
  default:
    printf("UNKNOWN=%d", object);
  }
}

extern void
dump_cell_type(cell_t cellt){
 switch(cellt){
  case FLOOR:
    printf("Floor Cell\n");
    break;
  case JAIL1:
    printf("Jail Cell, Team 1\n");
    break;
  case JAIL2:
    printf("Jail Cell, Team 2\n");
    break;
  case HOME1:
    printf("Home Cell, Team 2\n");
    break;
  case HOME2:
    printf("Home Cell, Team 2\n");
    break;
  case WALL:
    printf("Wall Cell\n");
    break;
  case IWALL:
    printf("Immutable wall Cell\n");
    break;
  default:
    printf("UNKNOWN=%d", cellt);
 }
}

extern void
proto_dump_mt(Proto_Msg_Types type)
{
  switch (type) {
  case PROTO_MT_REQ_BASE_RESERVED_FIRST: 
    fprintf(stderr, "PROTO_MT_REQ_BASE_RESERVED_FIRST");
    break;
  case PROTO_MT_REQ_BASE_HELLO: 
    fprintf(stderr, "PROTO_MT_REQ_BASE_HELLO");
    break;
  case PROTO_MT_REQ_BASE_MOVE: 
    fprintf(stderr, "PROTO_MT_REQ_BASE_MOVE");
    break;
  case PROTO_MT_REQ_BASE_CINFO:
    fprintf(stderr, "PROTO_MT_REQ_BASE_CINFO");    
    break;
  case PROTO_MT_REQ_BASE_DUMP:
    fprintf(stderr, "PROTO_MT_REQ_BASE_DUMP");    
    break;
  case PROTO_MT_REQ_BASE_GOODBYE: 
    fprintf(stderr, "PROTO_MT_REQ_BASE_GOODBYE");
    break;
  case PROTO_MT_REQ_BASE_RESERVED_LAST: 
    fprintf(stderr, "PROTO_MT_REQ_BASE_RESERVED_LAST");
    break;
  case PROTO_MT_REP_BASE_RESERVED_FIRST: 
    fprintf(stderr, "PROTO_MT_REP_BASE_RESERVED_FIRST");
    break;
  case PROTO_MT_REP_BASE_HELLO: 
    fprintf(stderr, "PROTO_MT_REP_BASE_HELLO");
    break;
  case PROTO_MT_REP_BASE_MOVE:
    fprintf(stderr, "PROTO_MT_REP_BASE_MOVE");
    break;
  case PROTO_MT_REP_BASE_CINFO:
    fprintf(stderr, "PROTO_MT_REP_BASE_CINFO");    
    break;
  case PROTO_MT_REP_BASE_DUMP:
    fprintf(stderr, "PROTO_MT_REP_BASE_DUMP");    
    break;
  case PROTO_MT_REP_BASE_GOODBYE:
    fprintf(stderr, "PROTO_MT_REP_BASE_GOODBYE");
    break;
  case PROTO_MT_REP_BASE_RESERVED_LAST: 
    fprintf(stderr, "PROTO_MT_REP_BASE_RESERVED_LAST");
    break;
  case PROTO_MT_EVENT_BASE_RESERVED_FIRST: 
    fprintf(stderr, "PROTO_MT_EVENT_BASE_RESERVED_LAST");
    break;
  case PROTO_MT_EVENT_BASE_UPDATE: 
    fprintf(stderr, "PROTO_MT_EVENT_BASE_UPDATE");
    break;
  case PROTO_MT_EVENT_BASE_MOVE: 
    fprintf(stderr, "PROTO_MT_EVENT_BASE_MOVE");
    break;
  case PROTO_MT_EVENT_BASE_SERVER_QUIT: 
    fprintf(stderr, "PROTO_MT_EVENT_BASE_SERVER_QUIT");
    break;
  case PROTO_MT_EVENT_BASE_PLAYER_JOIN: 
    fprintf(stderr, "PROTO_MT_EVENT_BASE_PLAYER_JOIN");
    break;
  case PROTO_MT_EVENT_BASE_RESERVED_LAST: 
    fprintf(stderr, "PROTO_MT_EVENT_BASE_RESERVED_LAST");
    break;
  default:
    fprintf(stderr, "UNKNOWN=%d", type);
  }
}

extern void
proto_dump_pstate(Proto_Player_State *ps)
{
  int v0, v1, v2, v3;
  
  v0 = ntohl(ps->v0.raw);
  v1 = ntohl(ps->v1.raw);
  v2 = ntohl(ps->v2.raw);
  v3 = ntohl(ps->v3.raw);

  fprintf(stderr, "v0=x0%x v1=0x%x v2=0x%x v3=0x%x\n",
	  v0, v1, v2, v3);
}

extern void
proto_dump_gstate(Proto_Game_State *gs)
{
  int v0, v1, v2;

  v0 = ntohl(gs->v0.raw);
  v1 = ntohl(gs->v1.raw);
  v2 = ntohl(gs->v2.raw);

  fprintf(stderr, "v0=0x%x v1=0x%x v2=0x%x\n",
	  v0, v1, v2);
}

extern void
proto_dump_msghdr(Proto_Msg_Hdr *hdr)
{
  fprintf(stderr, "ver=%d type=", ntohl(hdr->version));
  proto_dump_mt(ntohl(hdr->type));
  fprintf(stderr, " sver=%llx", ntohll(hdr->sver.raw));
  fprintf(stderr, " pstate:");
  proto_dump_pstate(&(hdr->pstate));
  fprintf(stderr, " gstate:"); 
  proto_dump_gstate(&(hdr->gstate));
  fprintf(stderr, " blen=%d\n", ntohl(hdr->blen));
}
