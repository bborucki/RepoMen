#include <string.h>
#include "player.h"
#include "cell.h"
#include "types.h"
#include "objectmap.h"
#include "playerlist.h"

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
player_do_tagged(){}

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
player_move(dir_t dir, Player *p, ObjectMap *o, Map *m){
  int ret = 0;
  int dim = o->dim;
  Player *otherPlayer;
  int x,y,nx,ny;
  
  x = p->pcell->x;
  y = p->pcell->y;
  
  if(dir == UP){
    nx = x-1;
    ny = y;
  }else if(dir == DOWN){
    nx = x+1;
    ny = y;
  }else if(dir == LEFT){
    nx = x;
    ny = y-1;
  }else{
    nx = x;
    ny = y+1;
  }

  if((ret = objectmap_validate_move(nx,ny,p,o)) > 0){
    if(ret == 2)
      objectmap_tagHandler(p,otherPlayer);

    //took out the cell_create (I think this was old...) -chris
    //need to also change player state and check if 
    //picking up flag or a shovel and check if digging

    objectmap_remove_player(p->pcell->x,p->pcell->y, o);
    p->pcell = o->objects[nx*dim + ny];
    o->objects[nx*dim+ny]->player = p;
    return 1;
  }
  
  return 0;
}

//Need to ensure that the cell gets into the objectmap

