#include <string.h>
#include "player.h"
#include "cell.h"
#include "types.h"
#include "objectmap.h"
#include <stdio.h>

extern void
player_dump(Player *p){
  printf("ID = %d\n", p->id);
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
player_find_empty_home(Player* p, team_t t, ObjectMap *o, int playerid){
  int x,y,idx;
  if(t == TEAM1){
    for(x = 91; x<=109; x++){
      for(y = 2; y<=11; y++){
	idx = x*(o->dim)+y;
	if(o->objects[idx]->type == HOME1 && o->objects[idx]->player == NULL
	   && o->objects[idx]->obj == NONE){
	  p->pcell = o->objects[idx];
	  p->pcell->x = x;
	  p->pcell->y = y;
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
	  p->pcell->x = x;
	  p->pcell->y = y;
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
    nx = x-1;
    ny = y;
  } else if (dir == DOWN){
    nx = x+1;
    ny = y;
  } else if (dir == LEFT){
    nx = x;
    ny = y-1;
  } else {
    nx = x;
    ny = y+1;
  }

  if((ret = objectmap_validate_move(nx,ny,p,o)) > 0){
    if(ret == 2)
      player_tagHandler(otherPlayer,o,g);
    nidx = nx*dim + ny;
    //return code 3 for ret is digging

    if(p->state == SAFE)
      if(o->objects[nidx]->type == FLOOR)
	p->state = FREE;

    if(p->state == FREE){
      if(o->objects[nidx]->type == HOME1 && p->team == TEAM1)
	p->state = SAFE;
      else if(o->objects[nidx]->type == HOME2 && p->team == TEAM2)
	p->state = SAFE;
    }

    objectmap_remove_player(p->pcell->x,p->pcell->y, o);
    p->pcell = o->objects[nidx];
    o->objects[nidx]->player = p;
    return 1;
  }
  
  return 0;
}

//Need to ensure that the cell gets into the objectmap

