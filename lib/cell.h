#ifndef __CELL_H__
#define __CELL_H__

#include "objectmap.h"
#include "map.h"
#include "player.h"
#include "types.h"

struct Cell_ {
  cell_t type;
  team_t team;
  unsigned char x;
  unsigned char y;
  object_t obj;
  Player *player;
  char playerid;
};

extern int cell_create(Map *m, Cell *c, int x, int y);
extern int cell_print_type(Cell *c);

#endif
