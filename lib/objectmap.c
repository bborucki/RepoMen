#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <time.h>
#include "math.h"
#include "objectmap.h"
#include "map.h"
#include "cell.h"
#include "player.h"
#include "playerlist.h"

extern int
objectmap_remove_player(int x, int y, ObjectMap *o){
  int idx = x*(o->dim)+y;
  o->objects[idx]->player = NULL;

  return 1;
}

extern int
objectmap_tagHandler(Player *tager, Player *tagee){
  return 0;
}
 
extern int
objectmap_validate_move(int x, int y, Player *p, ObjectMap *o){
  int idx, dim;
  Player *otherPlayer;

  dim = o->dim;
  
  if(x > dim || y > dim)
    return 0;
  if(p->state == JAILED)
    return 0;
  
  idx = x*dim+y;

  if(o->objects[idx] != NULL){
    if((otherPlayer = o->objects[idx]->player) != NULL)
      return player_is_taggable(p,otherPlayer);
    if(o->objects[idx]->type == IWALL)
      return 0;
    if(o->objects[idx]->type == WALL && p->shovel == NONE)
      return 0;
  }
  return 1;
}

extern int
objectmap_create(Map *m, ObjectMap *o){
  int seed;
  int idx;
  int i,j;
  int x,y;
  int dim = m->dim;

  
  for(i = 0; i < dim; i++){
    for(j = 0; j < dim; j++){
      o->objects[i*dim + j] = (Cell *)malloc(sizeof(Cell));
      cell_create(m,o->objects[i*dim + j], i, j);
    }
  }
  
  
  seed = time(NULL);
  srand(seed);

  //need to check that location randomly picked is valid
  //ie. not a wall

  x = rand()%(dim/2);
  y = rand()%dim;
  idx = x*dim+y;

  // while wall or iwall, search for new rand cell, otherwise put it in that one
  //  o->objects[idx]->obj = FLAG1;
  //same for flag 2
  x = rand()%(dim/2)+dim/2;
  y = rand()%dim;
  idx = x*dim+y;
  // use logic for finding rand cell
  //  if(o->objects[idx]->type == WALL || o->objects[idx]->type == IWALL){
  //    objectmap_remove_player(x,y,o);
  //  }
  //  o->objects[x*dim+y]->obj = FLAG2;
  
  x = 109;
  y = 2;
  idx = x*dim+y;
  printf("x = %d, y = %d \n", x, y);
  o->objects[idx]->obj = SHOVEL1;

  x = 109;
  y = 197;
  idx = x*dim+y;
  printf("x = %d, y = %d \n", x, y);
  o->objects[idx]->obj = SHOVEL2;

  o->numCells = 4;
  o->numPlayers = 0;
  o->dim = dim;
  
  return 1;
}
