#ifndef __CELL_H__
#define __CELL_H__

#include "types.h"
#include "gamestate.h"
#include "objectmap.h"
#include "map.h"
#include "player.h"

struct Cell_ {
  cell_t type;
  team_t team;
  int x;
  int y;
  int idx;
  object_t obj;
  Player *player;
  char playerid;
};

extern int cell_create(Map *m, Cell *c, int x, int y);
extern int cell_print_type(Cell *c);
extern int cell_dump(Cell *c);

#endif
