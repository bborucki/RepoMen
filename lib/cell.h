#ifndef __CELL_H__
#define __CELL_H__

#include "map.h"

typedef struct Cell{
  int type;
  int team;
  int occupied;
  int x;
  int y;
  int obj1;
  int obj2;
} Cell;

#define OCCUPIED 1
#define UNOCCUPIED 0

extern int cell_create(Map *m, Cell *c, int x, int y);
extern int cell_print_type(Cell *c);

#endif
