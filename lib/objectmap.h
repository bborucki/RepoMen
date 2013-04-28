#ifndef __OBJECT_MAP__
#define __OBJECT_MAP__

#include "types.h"
#include "cell.h"
#include "map.h"
#include "player.h"

#define MAX_PLAYERS 198

struct ObjectMap_ {
  Cell **objects;
  int numCells;
  int numPlayers;
  int dim;
};

extern ObjectMap * objectmap_create(Map *m);
extern int objectmap_validate_move(int x, int y, Player *p, ObjectMap *o);
extern int objectmap_tagHandler(Player *tager, Player *tagee);
extern int objectmap_reset_cell(int x, int y, ObjectMap *o);

#endif
