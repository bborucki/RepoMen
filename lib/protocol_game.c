#ifndef __DAGAME_PROTOCOL_SERVER_H__
#define __DAGAME_PROTOCOL_SERVER_H__
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


static Map* Server_Map;
Cell *objects[MAX_LINE][COLUMN_MAX]; //2d array of map
Player *players[MAX_PLAYERS];
int playernext;
int numobjects;

int
move_valid(int x, int y, Player* player){
  if(x > MAX_LINE || y > COLUMN_MAX)
    return 0;

  if(objects[x][y] != NULL){
    if(objects[x][y]->occupied == OCCUPIED)
      return 0;
    if(objects[x][y]-> type == IWALL)
      return 0;
    if(objects[x][y]-> type == WALL && player->object != SHOVEL)
      return 0;
  }

}

int
initialize_map(){

}


int 
make_move(int x, int y){

}


int
drop_object(Player* p){
  

}
