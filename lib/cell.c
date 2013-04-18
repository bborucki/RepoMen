#include <stdio.h>
#include "map.h"
#include "cell.h"

extern int
player_create(Player* p, int playerid, int playerteam, Cell* cell){
  p->id = playerid;
  p->team = playerteam;
  p->pcell = cell;
  p->flag = NONE;
  p->shovel = NONE;
  p->state = SAFE;
}

int
isMutable(Map* m, int x, int y){
  if(x == LINE_MAX - 1 || x == 0 || y == COLUMN_MAX -1 ||
     y == 0)
    return 0;
  else if(m->maze[x*((m->dim)-1)+y] == 'h' || 
	  m->maze[x*((m->dim)+1)+y] == 'h' ||
	  m->maze[x*((m->dim))+y-1] == 'h' ||
	  m->maze[x*((m->dim))+y+1] == 'h' ||
	  m->maze[x*((m->dim))+y-1] == 'j' ||
	  m->maze[x*((m->dim))+y+1] == 'j' ||
	  m->maze[x*((m->dim)-1)+y] == 'j' ||
	  m->maze[x*((m->dim)+1)+y] == 'j'){
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
  case 'h':
    if(x < COLUMN_MAX/2)
      return HOME1;
    else
      return HOME2;
  case 'j':
    if(x < COLUMN_MAX/2)
      return JAIL1;
    else
      return JAIL2;
  case ' ':
    return FLOOR;
  default:
    return -1;
  }
}

int
getTeam(int x, int y){
  if(x > COLUMN_MAX/2)
    return TEAM2;
  return TEAM1;
}

extern int
cell_create(Map *m, Cell *c, int x, int y){
  c->x = x;
  c->y = y;
  c->team = getTeam(x,y);
  c->occupied = UNOCCUPIED;
  c->obj = NONE;
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

