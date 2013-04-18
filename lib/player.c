#include "player.h"
#include "cell.h"
#include "types.h"
#include "objectmap.h"
#include "playerlist.h"

int
player_drop_object(){}

extern int
player_move(int x, int y, Player *p, ObjectMap *o, PlayerList *pl){
  int dim = o->dim;
  
  if(validate_move(x,y,p,o)){
    if(objectmap_is_taggable(x,y,p,o,pl)){
      tagHandler(p,pl[o->objects[x*dim + y]->player->id]);
    }
    p->pcell = o->objects[x*dim + y];
    o->objects[x*dim+y]->playerid = p->id;
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
