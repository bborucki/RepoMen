#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>
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

char** load_maze(char* mappath)
{
  char* buf = malloc(COLUMN_MAX);
  char** out = malloc(LINE_MAX);
  int i = 0;
  if(!read_map(MAP_NAME))
    fprintf(stderr, "error reading map at path %s\n", mappath); 
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

int get_cell_type(Map *m,int x, int y)
{
  return m->maze[x-1][y-1];
}

int read_map(const char* mappath)
{
  if((fp = fopen(mappath, "r")) == NULL)
    return 0;
  return 1;
}

int map_dump(const char* mappath)
{
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


int map_num_home(const char* mappath, int team)
{
  char* buf = malloc(LINE_MAX);
  int i;
  int num = 0;
  int cap;
  if(team == 1)
    cap = 0;
  else
    cap = 1;

  if(!read_map(mappath)){
    fprintf(stderr,"could not read map at path %s",mappath);
    return 0;
  }
  
  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(; buf != '\0'; buf++){
      if(cap && *buf == 'H')
	num++;
      else if(!cap && *buf == 'h')
	num++;
    }  
  }
  return num;
}

int map_num_jail(const char* mappath, int team)
{
  char* buf = malloc(LINE_MAX);
  int i;
  int num = 0;
  int cap;
  if(team == 1)
    cap = 0;
  else
    cap = 1;

  if(!read_map(mappath)){
    fprintf(stderr,"could not read map at path %s",mappath);
    return 0;
  }
  
  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(; buf != '\0'; buf++){
      if(cap && *buf == 'J')
	num++;
      else if(!cap && *buf == 'j')
	num++;
    }  
  }
  return num;
}


int map_num_wall(const char* mappath)
{
  char* buf = malloc(LINE_MAX);
  int num = 0;
  
  if(!read_map(mappath)){
    fprintf(stderr,"could not read map at path %s",mappath);
    return 0;
  }
  
  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(; buf != '\0'; buf++){
	if(*buf == '#')
	  num++;
    }
  }
  return num;
}

int map_num_floor(const char* mappath)
{
  char* buf = malloc(LINE_MAX);
  int num = 0;
  
  if(!read_map(mappath)){
    fprintf(stderr,"could not read map at path %s",mappath);
    return 0;
  }
  
  while(fgets(buf,LINE_MAX,fp) != NULL){
    for(; buf != '\0'; buf++){
      if(*buf == ' ' || *buf == 'j' || *buf == 'J'||	\
	   *buf == 'h' || *buf == 'H'){
	  
	  num++;
	}
    } 
  }
  return num;
}



extern void load_map(Map* m)
{
  m->numhome1 = map_num_home(MAP_NAME, 1);
  m->numhome2 = map_num_home(MAP_NAME, 2);
  m->numwall = map_num_wall(MAP_NAME);
  m->numfloor = map_num_floor(MAP_NAME);
  m->numjail1 = map_num_jail(MAP_NAME, 1);
  m->numjail2 = map_num_jail(MAP_NAME, 2);
  m->dim = COLUMN_MAX;
  m->maze = (char**)load_maze(MAP_NAME);
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
