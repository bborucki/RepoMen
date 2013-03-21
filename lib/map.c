#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "map.h"
#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_client.h"

FILE *fp;

typedef struct Cell{
  int type;
  int team;
  int occupied;
  int x;
  int y;
  int obj1;
  int obj2;
} Cell;

char** load_maze()
{
  char* buf = malloc(COLUMN_MAX);
  char** out = malloc(LINE_MAX);
  int i = 0;
  while(fgets(buf,COLUMN_MAX,fp) != NULL){
    out[i] = strcpy(out[i],buf);
    i++;
  }  
  return out;
}

Cell* make_cell(int ux, int uy)
{
  Cell* c = (Cell*) malloc(sizeof(Cell));
  c->x = ux;
  c->y = uy;
  c->team = 0;
  c->occupied = UNOCCUPIED;
  c->obj1 = 0;
  c->obj2 = 0;
  c->type = get_cell_type(ux,uy);
  return c;
}

int get_cell_type(Map *m,int x, int y){
  return m->maze[x-1][y-1];
}

int read_map(const char* mappath){
  if((fp = fopen(mappath, "r")) == NULL){
    fprintf(stderr, "ERRNO: %d\n", errno);
    return 0;
  }
  return 1;
}

int map_dump(const char* mappath){
  char* buf = malloc(LINE_MAX);
  char* out = malloc(LINE_MAX);
  int i,j;
  if(!read_map(mappath)){
    fprintf(stderr, "could not read map with path %s\n",mappath);
    return 0;
  }
  while(fgets(buf,LINE_MAX, fp) != NULL){
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

int map_num_home(int team){
  char buf[LINE_MAX];
  int i;
  int num = 0;
  char c = 'H';

  if(team == 1)
    c = 'h';
  
  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(i=0; i<LINE_MAX; i++){
      if(buf[i] == c){
	printf("%c", buf[i]);
	num++;
      }
    }  
  }
  return num;
}

int map_num_jail(int team){
  char* buf = malloc(LINE_MAX);
  int i;
  int num = 0;
  int cap;
  if(team == 1)
    cap = 0;
  else
    cap = 1;
  
  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(i=0; i<LINE_MAX; i++){
      if(cap && buf[i] == 'J')
	num++;
      else if(!cap && buf[i] == 'j')
	num++;
    }  
  }
  return num;
}

int map_num_wall()
{
  char* buf = malloc(LINE_MAX);
  int num = 0;
  int i;

  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(i=0; i<LINE_MAX; i++){
	if(buf[i] == '#')
	  num++;
    }
  }
  return num;
}

int map_num_floor()
{
  char* buf = malloc(LINE_MAX);
  int num = 0;
  int i;
  
  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(i=0; i<LINE_MAX; i++){
      if(buf[i] == ' ' || buf[i] == 'j' || buf[i] == 'J'||	\
	   buf[i] == 'h' || buf[i] == 'H'){
	  
	  num++;
	}
    } 
  }
  return num;
}

extern int
load_map(Map* m)
{
  if(!read_map(MAP_NAME)){
    fprintf(stderr,"could not read map at path %s\n",MAP_NAME);
    return 0;
  }
  m->numhome1 = map_num_home(1);
  m->numhome2 = map_num_home(2);
  m->numjail1 = map_num_jail(1);
  m->numjail2 = map_num_jail(2);
  m->numwall = map_num_wall();
  m->numfloor = map_num_floor();
  m->dim = COLUMN_MAX;
  m->maze = (char**)load_maze();
  return 1;
}

/*
extern
Cell * map_cinfo(char* mappath, int x, int y)
{
  int i;
  for(i = 0; i<num_occupied; i++){
    if((occupied_cells[i]->x) == x && (occupied_cells[i]->y == y))
      return occupied_cells[i];
  }
  return make_cell(x, y);
}
*/
