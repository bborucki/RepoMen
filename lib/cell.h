#ifndef __CELL_H__
#define __CELL_H__

#include "map.h"

typedef enum {FLOOR, JAIL1, JAIL2, HOME1, HOME2, WALL, IWALL} cell_t;

typedef enum {SAFE, JAILED, FREE} state_t;

typedef enum {NONE, FLAG, SHOVEL} object_t;

typedef enum {RED, GREEN} team_t;



typedef struct Cell{
  cell_t type;
  team_t team;
  int occupied;
  int x;
  int y;
  object_t obj1;
} Cell;

typedef struct Player{
  int id;
  team_t team;
  object_t object;
  state_t state;
  Cell* pcell;
} Player;


#define OCCUPIED 0
#define UNOCCUPIED 1

extern int cell_create(Map *m, Cell *c, int x, int y);
extern int cell_print_type(Cell *c);
player_create(Player* p, int playerid, int playerteam, Cell* cell);

#endif
