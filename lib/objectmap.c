#include <stdlib.h>
#include <strings.h>
#include "objectmap.h"
#include "map.h"
#include "cell.h"
#include "player.h"
#include "playerlist.h"

int
objectmap_tag_handler(){}

extern int
objectmap_is_taggable(int x, int y, Player *p, ObjectMap *o, PlayerList *pl){
  int dim = o->dim;

  if(pl[o->objects[x*dim + y]->playerid] != NULL){
    if(pl[o->objects[x*dim + y]->playerid]-> team == p->team){
      return 0;
    }
    if(o->objects[x*dim + y]->team != pl[o->objects[x*dim+y]->playerid]->team)
      return 1;
  }  
}

extern int
objectmap_validate_move(int x, int y, Player *p, ObjectMap *o, PlayerList *pl){
  int idx, dim;

  dim = o->dim;
  x--; y--; //assumes offset for dimensions starting at 1
  
  if(x > dim || y > dim)
    return 0;
  if(p->state == JAILED)
    return 0;
  
  idx = x*dim+y;

  if(o->objects[idx] != NULL){
    if(o->objects[idx]->player != NULL)
      return objectmap_is_taggable(x,y,p,o,pl);
    if(o->objects[idx]->type == IWALL)
      return 0;
    if(o->objects[idx]->type == WALL && p->shovel != SHOVEL)
      return 0;
  }
  return 1;
}

extern ObjectMap *
objectmap_create(Map *m){
  ObjectMap *o = (ObjectMap *)malloc(sizeof(ObjectMap));
  int i, j;
  int dim = m->dim;

  bzero(o, sizeof(ObjectMap));

  o->objects = (Cell **)malloc(sizeof(Cell *)*dim*dim);
  bzero(o->objects, sizeof(o->objects));

  for(i = 0; i < dim; i++){
    for(j = 0; j < dim; j++){
      o->objects[i*dim + j] = (Cell *)malloc(sizeof(Cell));
      cell_create(m,o->objects[i*dim + j], i, j);
    }
  }

  o->numPlayers = 0;
  o->dim = dim;
  
  return 0;
}
