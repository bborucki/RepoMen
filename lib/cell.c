#include <stdio.h>
#include "map.h"
#include "cell.h"
#include "types.h"
#include "player.h"

extern int
cell_dump(Cell *c){
  cell_print_type(c);
  dump_team(c->team);
  printf("Location: %d,%d\n", c->x, c->y);
  dump_object(c->obj);
  printf("Player ID: %d\n", (int)(c->playerid));
  return 1;
}

int
isMutable(Map* m, int x, int y){
  int dim = m->dim;
  int w = x*dim+y;
  int up = ((x-1)*dim)+y;
  int down = ((x+1)*dim)+y;

  if(x == dim-1 || x == 0 || y == dim-1 || y == 0)
    return 0;
  else if(m->maze[up] == 'h' || 
	  m->maze[down] == 'h' ||
	  m->maze[w-1] == 'h' ||
	  m->maze[w+1] == 'h' ||
	  m->maze[w-1] == 'j' ||
	  m->maze[w+1] == 'j' ||
	  m->maze[up] == 'j' ||
	  m->maze[down] == 'j'){
    return 0;
  }
  return 1;
}

int
cell_get_type(Map *m, int x, int y){
  char c;
  c = m->maze[x*(m->dim)+y];
  switch(c){
  case '#':
    if(isMutable(m,x,y))
      return WALL;
    else
      return IWALL;
  case ' ':
    return FLOOR;
  case 'h':
      return HOME1;
  case 'H':
      return HOME2;
  case 'j':
      return JAIL1;
  case 'J':
      return JAIL2;
  default:
    return -1;
  }
}

int
getTeam(int x, int y, int dim){
  if(x > dim/2) //may need to use floor() or ceil()
    return TEAM2;
  return TEAM1;
}

extern int
cell_create(Map *m, Cell *c, int x, int y){
  c->x = x;
  c->y = y;
  c->idx = x*m->dim + y;
  c->team = getTeam(x,y,m->dim);
  c->player = NULL;
  c->obj = NONE;
  c->type = cell_get_type(m,x,y);
  c->playerid = 0;
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

