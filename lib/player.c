#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "player.h"
#include "cell.h"
#include "types.h"
#include "objectmap.h"

#define DEBUG_MAP 0

extern void
player_dump(Player *p){
  printf("ID = %d\n", p->id);
  printf("Location = (%d,%d)\n", p->x,p->y);
  dump_team(p->team);
  dump_state(p->state);
  dump_object(p->shovel);
  dump_object(p->flag);
}

extern int
player_find_next_id(Player **players){
  int i;
  for(i = 0; i<MAX_PLAYERS; i++){
    if(players[i] == NULL)
      return i;
  }
  return -1;
}

extern int 
player_obj_pickup(Player* p, ObjectMap *o, Gamestate *g){
  int idx,x,y;
  team_t t;
  x = p->x;
  y = p->y;
  idx = x*(o->dim)+y;
  if(o->objects[idx] == NULL || o->objects[idx]->obj == NONE)
    return -1;
  if(o->objects[idx]->obj == FLAG1 ||o->objects[idx]->obj == FLAG2){
    p->flag = o->objects[idx]->obj;
    o->objects[idx]->obj = NONE;
    gamestate_remove_cell(g,x,y);
    return p->flag;
  } else if(o->objects[idx]->obj == SHOVEL1 || o->objects[idx]->obj == SHOVEL2){
    p->shovel = o->objects[idx]->obj;
    o->objects[idx]->obj = NONE;
    gamestate_remove_cell(g,x,y);
    return p->shovel;
  }
  return -1;
}

extern int 
player_find_empty_home(Player* p, team_t t, ObjectMap *o, int playerid){
  int x,y,idx;

  if(DEBUG_MAP){
    x = 2;
    y = 2;
    idx = x*(o->dim)+y;
    p->pcell = o->objects[idx];
    p->pcell->x = x;
    p->pcell->y = y;
    p->x = x;
    p->y = y;
    p->id = playerid;
    p->team = t;
    p->shovel = NONE;
    p->flag = NONE;
    p->state = SAFE;
    o->objects[idx]->player = p;
    return 1;	  
  }
  
  if(t == TEAM1){
    for(x = 91; x<=109; x++){
      for(y = 2; y<=11; y++){
	idx = x*(o->dim)+y;
	if(o->objects[idx]->type == HOME1 && o->objects[idx]->player == NULL
	   && o->objects[idx]->obj == NONE){
	  p->pcell = o->objects[idx];
	  p->x = x;
	  p->y = y;
	  p->id = playerid;
	  p->team = t;
	  p->shovel = NONE;
	  p->flag = NONE;
	  p->state = SAFE;
	  o->objects[idx]->player = p;
	  return 1;
	}
      }
    }
  }
  else{
    for(x = 91; x<=109; x++){
      for(y = 188;y<=197; y++){
	idx = x*(o->dim)+y;
	if(o->objects[idx]->type == HOME2 && o->objects[idx]->player == NULL
	   && o->objects[idx]->obj == NONE){
	  p->pcell = o->objects[idx];
	  p->x = x;
	  p->y = y;
	  p->id = playerid;
	  p->team = t;
	  p->shovel = NONE;
	  p->flag = NONE;
	  p->state = SAFE;
	  o->objects[idx]->player = p;
	  printf("y = %d\n", y);
	  return 1;
	}
      }
    }
  }
  return 0;
}
int
player_drop_object(){}


extern int
player_tagHandler(Player *tagee, ObjectMap *o, Gamestate *g){
  int x,y,px,py, idx;
  team_t t;
  t = tagee->team;

  idx = x*(o->dim)+y;

  if(tagee->shovel != NONE){
    objectmap_place_shovel(o,g,tagee->team);
  }
  if(tagee->flag != NONE){
    if(tagee == TEAM1)
      o->objects[idx]->obj = FLAG1;
    else
      o->objects[idx]->obj = FLAG2;
  }

  if(t == TEAM1){
    for(x = 91; x<=109; x++){
      for(y = 102; y<=110; y++){
	idx = x*(o->dim)+y;
	if(o->objects[idx]->type == JAIL2 && o->objects[idx]->player == NULL){
	  tagee->pcell = o->objects[idx];
	  tagee->pcell->x = x;
	  tagee->pcell->y = y;
	  tagee->shovel = NONE;
	  tagee->flag = NONE;
	  tagee->state = JAILED;
	  o->objects[idx]->player = tagee;
	  return 1;
	}
      }
    }
  }
  else{
    for(x = 91; x<=109; x++){
      for(y = 90;y<=98; y++){
	idx = x*(o->dim)+y;
	if(o->objects[idx]->type == JAIL1 && o->objects[idx]->player == NULL){
	  tagee->pcell = o->objects[idx];
	  tagee->pcell->x = x;
	  tagee->pcell->y = y;
	  tagee->shovel = NONE;
	  tagee->flag = NONE;
	  tagee->state = JAILED;
	  o->objects[idx]->player = tagee;
	  return 1;
	}
      }
    }
  }
  objectmap_remove_player(tagee->pcell->x,tagee->pcell->y, o);
      
}


extern int
player_is_taggable(Player *tager, Player *tagee){
  if(tager->state != JAILED && tagee->state == FREE){
    if(tager->team == tagee->team)
      return 0;
    return 2;
  }
  return 0;
}

extern int
player_move(dir_t dir, Player *p, ObjectMap *o, Gamestate *g){
  int ret = 0;
  int dim = o->dim;
  Player *otherPlayer;
  int x,y,nx,ny,nidx;
  
  x = p->pcell->x;
  y = p->pcell->y;
  
  if(dir == UP){
    nx = x;
    ny = y-1;
  } else if (dir == DOWN){
    nx = x;
    ny = y+1;
  } else if (dir == LEFT){
    nx = x-1;
    ny = y;
  } else {
    nx = x+1;
    ny = y;
  }

  if((ret = objectmap_validate_move(nx,ny,p,o)) > 0){
    if(ret == 2)
      player_tagHandler(otherPlayer,o,g);
    if(ret == 3)
      objectmap_digger(p,nx,ny,o,g);

    nidx = nx*dim + ny;

    if(p->state == SAFE && o->objects[nidx]->type == FLOOR)
      p->state = FREE;

    if(p->state == FREE){
      if(o->objects[nidx]->type == HOME1 && p->team == TEAM1){
	p->state = SAFE;
	if(p->flag == FLAG1)
	  flag1home1 = 1;
	if(p->flag == FLAG2)
	  flag2home1 = 1;
	numplayershome1++;
      }
      else if(o->objects[nidx]->type == HOME2 && p->team == TEAM2){
	p->state = SAFE;
	if(p->flag == FLAG1)
	  flag1home2 = 1;
	if(p->flag == FLAG2)
	  flag2home2 = 1;
	numplayershome2++;
      }
    }

    objectmap_remove_player(p->pcell->x,p->pcell->y, o);
    p->pcell = o->objects[nidx];
    o->objects[nidx]->player = p;
    return 1;
  }
  else
    return 0;
}

//Need to ensure that the cell gets into the objectmap

extern Player *
player_create(){
  Player *p;
  p = (Player *)malloc(sizeof(Player));
  bzero(p, sizeof(Player));
  return p;
}
