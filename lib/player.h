#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cell.h"
#include "objectmap.h"
#include "types.h"


struct Player_ {
  int id;
  team_t team;
  object_t shovel;
  object_t flag;
  state_t state;
  Cell *pcell;
};

extern int player_create(Player *p, int playerid, int playerteam, Cell *cell);
extern int player_move(int x, int y, Player *p, ObjectMap *o);

#endif
