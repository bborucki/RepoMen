#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <time.h>
#include "math.h"
#include "objectmap.h"
#include "map.h"
#include "cell.h"
#include "player.h"
#include "gamestate.h"

#define DEBUG_MAP 0

extern int
objectmap_remove_player(int x, int y, ObjectMap *o){
  int idx = x*(o->dim)+y;
  o->objects[idx]->player = NULL;

  return 1;
}

extern int 
objectmap_reset_cell(int x, int y, ObjectMap *o){
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

  printf("celltype = %d\n", o->objects[idx]->type);
  if(o->objects[idx] != NULL){
    if((otherPlayer = o->objects[idx]->player) != NULL)
      return player_is_taggable(p,otherPlayer);
    else if(o->objects[idx]->type == IWALL){
      printf("hit an iwall\n");
      return 0;
    } else if(o->objects[idx]->type == WALL){
      if(p->shovel != NONE){
	printf("hit a wall with shovel\n");
	return 3;
      } else{
	return -1;
      }
    } else{
      return 1;
    }
  }
  return 0;
}

extern int
objectmap_place_shovel(ObjectMap *o, Gamestate *g, team_t team){
  int x,y,idx;
  int dim = o->dim;

  if(team == TEAM1){
    x = 109;
    y = 2;
    idx = x*dim+y;
    printf("x = %d, y = %d \n", x, y);
    o->objects[idx]->obj = SHOVEL1;
  }
  else{
    x = 109;
    y = 197;
    idx = x*dim+y;
    printf("x = %d, y = %d \n", x, y);
    o->objects[idx]->obj = SHOVEL2;
  }
  gamestate_add_cell(g, o->objects[idx]);
}

extern int
objectmap_flag_visible(Player *p, ObjectMap *o){
  team_t t = p->team;
  int x,y,idx,dim,i;
  x = p->pcell->x - 5;
  y = p->pcell->y - 5;
  dim = o->dim;
  idx = x*dim+y;
  
    for(i=0;i<121;i++){
      if(t == TEAM1){
	if(o->objects[idx]->obj == FLAG1)
	  return idx;
      }
      if(t == TEAM2){
	if(o->objects[idx]->obj == FLAG2)
	  return idx;
      }
      idx++;
    }
    return -1;
}

extern int
objectmap_place_flag(ObjectMap *o, team_t t){
  int x,y,idx,dim;

  srand(time(NULL));
  dim = o->dim;

  x = rand() % dim/2;
  y = rand() % dim;
  idx = x*dim+y;
  
  
  while(o->objects[idx] == NULL || o->objects[idx]->type != FLOOR
	|| o->objects[idx]->team != t){
    x = rand() % dim/2;
    y = rand() % dim;
    idx = x*dim+y;
    
  }
  if(t == TEAM1)
    o->objects[idx]->obj = FLAG1;
  else
    o->objects[idx]->obj = FLAG2;

  return 1;

}
extern ObjectMap *
objectmap_create(Map *m, Gamestate *g){
  ObjectMap *o;

  int idx;
  int i,j;
  int x,y;
  int dim = m->dim;

  o = (ObjectMap *)malloc(sizeof(ObjectMap));
  bzero(o, sizeof(ObjectMap));

  o->objects = (Cell **)malloc(sizeof(Cell *)*dim*dim);
  bzero(o->objects, sizeof(o->objects));

  for(i = 0; i < dim; i++){
    for(j = 0; j < dim; j++){
      o->objects[i*dim + j] = (Cell *)malloc(sizeof(Cell));
      cell_create(m,o->objects[i*dim + j], i, j);
    }
  }

 
  o->numCells = 4;
  o->numPlayers = 0;
  o->dim = dim;

  if(!DEBUG_MAP){
    objectmap_place_flag(o,TEAM1);
    objectmap_place_flag(o,TEAM2);
    objectmap_place_shovel(o,g,TEAM1);
    objectmap_place_shovel(o,g,TEAM2);
  }
  
  return o;
}
