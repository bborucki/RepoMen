#ifndef __OBJECT_MAP__
#define __OBJECT_MAP__

#include "types.h"
#include "cell.h"
#include "map.h"
#include "player.h"
#include "gamestate.h"

#define MAX_PLAYERS 198

struct ObjectMap_ {
  Cell **objects;
  int numCells;
  int numPlayers;
  int dim;
};

extern ObjectMap * objectmap_create(Map *m, Gamestate *g);
extern int objectmap_validate_move(int x, int y, Player *p, ObjectMap *o);
extern int objectmap_reset_cell(int x, int y, ObjectMap *o);
extern int objectmap_place_shovel(ObjectMap *o, Gamestate *g, team_t team);
#endif
