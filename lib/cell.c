#include <stdio.h>
#include "map.h"
#include "cell.h"

int
cell_get_type(Map *m, int x, int y){
  return (int)m->maze[x*m->dim+y];
}

extern int
cell_create(Map *m, Cell *c, int x, int y){
  c->x = x;
  c->y = y;
  c->team = 0;
  c->occupied = UNOCCUPIED;
  c->obj1 = 0;
  c->obj2 = 0;
  c->type = cell_get_type(m,x,y);
  return 1;
}

extern int
cell_print_type(Cell *c){
  switch(c->type){
  case '#':
    printf("Wall\n");
    break;
  case ' ':
    printf("Floor\n");
    break;
  case 'h':
    printf("Home Team 1\n");
    break;
  case 'H':
    printf("Home Team 2\n");
    break;
  case 'j':
    printf("Jail Team 1\n");
    break;
  case 'J':
    printf("Jail Team 2\n");
    break;
  default:
    printf("\n");
    break;
  }



  return 1;
}

