#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "map.h"
#include "maze.h"
#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_client.h"

FILE *fp;

extern int 
make_cell(Map *m, Cell *c, int ux, int uy){
  c->x = ux;
  c->y = uy;
  c->team = 0;
  c->occupied = UNOCCUPIED;
  c->obj1 = 0;
  c->obj2 = 0;
  c->type = get_cell_type(m, ux,uy);
  return 1;
}

int 
get_cell_type(Map *m,int x, int y){
  return m->maze->data[x*m->maze->dim+y];
}

int 
map_dump(const char* mappath){
  int i,j;
  char *buf = (char *)malloc(MAX_LINE);
  char *out = (char *)malloc(MAX_LINE);

  if(!read_map(mappath)){
    fprintf(stderr, "could not read map with path %s\n",mappath);
    return 0;
  }
  while(fgets(buf,MAX_LINE, fp) != NULL){
    i = 0;
    /*
      while(i < TEAM_1_MAX){
      //  NYI designate team color;
      
      }
      while(i >= TEAM_1_MAX && i < TEAM_2_MAX){
      //  NYI designate team color;
      }
    */
    // this is temporary, but for now strcpy to out
    out = strcpy(out,buf);
    
    printf("%s",out);
  }
  
  return 1;
}

int
map_fill(Map *m){
  int i,j;
  int numHome1, numHome2, numJail1, numJail2;
  int numFloor, numWall;
  int dim;

  numHome1 = numHome2 = numJail1 = 0;
  numJail2 = numFloor = numWall = 0;

  dim = m->maze->dim;

  for(i=0; i < dim; i++){
    for(j=0; j < dim; j++){
      switch(m->maze->data[i*dim+j]){
      case '#':
	numWall++;
	break;
      case ' ':
	numFloor++;
	break;
      case 'h':
	numHome1++;
	numFloor++;
	break;
      case 'H':
	numHome2++;
	numFloor++;
	break;
      case 'j':
	numJail1++;
	numFloor++;
	break;
      case 'J':
	numJail2++;
	numFloor++;
	break;
      }
    }
  }
  
  m->numhome1 = numHome1;
  m->numhome2 = numHome2;
  m->numjail1 = numJail1;
  m->numjail2 = numJail2;
  m->numwall  = numWall;
  m->numfloor = numFloor;
}

extern int
map_init(Map* m){
  bzero(m,sizeof(m));
  m->maze = maze_init(MAP_NAME);
  map_fill(m);

  /*
    printf("h1: %d\n", m->numhome1);
    printf("h2: %d\n", m->numhome2);
    printf("j1: %d\n", m->numjail1);
    printf("j2: %d\n", m->numjail2);
    printf("wl: %d\n", m->numwall);
    printf("fl: %d\n", m->numfloor);
  */

  return 1;
}

/*
  extern Cell*
  map_cinfo(char* mappath, int x, int y)
  {
  int i;
  for(i = 0; i<num_occupied; i++){
  if((occupied_cells[i]->x) == x && (occupied_cells[i]->y == y))
  return occupied_cells[i];
  }
  return make_cell(x, y);
  }
*/
