#include <string.h>
#include "player.h"
#include "cell.h"
#include "types.h"
#include "objectmap.h"
#include "playerlist.h"

int
player_drop_object(){}

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
player_move(int x, int y, Player *p, ObjectMap *o, Map *m){
  int ret = 0;
  int dim = o->dim;
  Player *otherPlayer;

  if((ret = objectmap_validate_move(x,y,p,o)) > 0){
    if(ret == 2)
      objectmap_tagHandler(p,otherPlayer);
    
    objectmap_reset_cell(p->pcell->x,p->pcell->y, o, m);
    p->pcell = o->objects[x*dim + y];
    o->objects[x*dim+y]->player = p;
    return 1;
  }
  
  return 0;
}

extern int
player_create(Player *p, int playerid, int playerteam, Cell *cell){
  p->id = playerid;
  p->team = playerteam;
  p->pcell = cell;
  p->shovel = NONE;
  p->flag = NONE;
  p->state = SAFE;

  return 0;
}
