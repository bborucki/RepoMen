#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "map.h"
#include "cell.h"
#include "net.h"
#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_server.h"
#include "protocol_session.h"
#include "protocol_game.h"

/*starting to rethink map structure, not sure where to do this,
 * but 2d array of cell pointers seems to make sense.
 */

int
taggable(int x, int y, Player* tplayer){
  if(players[objects[x][y]->playerid] != NULL){
    if(players[objects[x][y]->playerid]->team == tplayer->team){
      return 0;
    }
    if(objects[x][y]->team != players[objects[x][y]->playerid]->team)
      return 1;
  }
  return 0;    
}

int
move_valid(int x, int y, Player *player, Map *m){
  x--; y--; //assumes offset for dimensions starting at 1

  if(x > m->dim || y > m->dim)
    return 0;
  if(player->state == JAILED){
    return 0;
  }
  
  if(objects[x][y] != NULL){
    if(objects[x][y]->player != NULL)
      return taggable(x, y, player);
    if(objects[x][y]-> type == IWALL)
      return 0;
    if(objects[x][y]-> type == WALL && player->shovel != SHOVEL)
      return 0;
  }
  return 1;
}

int
objects_init(Map *m){
  int i;
  int j;
  dim = m->dim;

  objects = malloc(sizeof(Cell *)*dim*dim);

  for(i = 0; i<dim; i++){
    for(j = 0; j<dim; j++){
      objects[i][j] = (Cell*)malloc(sizeof(int));
      cell_create(Server_Map,objects[i][j], i,j);
    }
  }

  return 0;
}

int
tagHandler(Player* tagger, Player* taggee){
  
}

int 
make_move(int x, int y, Player *player, Map *m){
  if(move_valid(x,y,player,m)){
    if(taggable(x,y,player)){
      tagHandler(player,players[objects[x][y]->playerid]);
    }
    player->pcell = objects[x][y];
    objects[x][y]->playerid = player->id;
    return 1;
  }
  return 0;
}


int
drop_object(Player* p){
  

}
