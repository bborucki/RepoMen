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

char** load_maze()
{
  char* buf = malloc(COLUMN_MAX);
  char** out = malloc(MAX_LINE*COLUMN_MAX);
  int i,j;
  while(fgets(buf,COLUMN_MAX,fp) != NULL){
    out[j] = malloc(strlen(buf));
    for(i=0;i<strlen(buf);i++)
      out[j][i] = buf[i];
    j++;
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
  return m->maze[x][y];
}

int read_map(const char* mappath){
  if((fp = fopen(mappath, "r")) == NULL){
    fprintf(stderr, "ERRNO: %d\n", errno);
    return 0;
  }
  return 1;
}

int map_dump(const char* mappath){
  char* buf = malloc(MAX_LINE);
  char* out = malloc(MAX_LINE);
  int i,j;
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

int map_num_home(char* buf, int team){
  int i;
  int num = 0;
  char c = 'H';

  if(team == 1)
    c = 'h';
  
  for(i=0; i<strlen(buf); i++){
    if(buf[i] == c){
      num++;
    }
  }  
  
  return num;
}

int map_num_jail(char* buf, int team){
  int i;
  int num = 0;
  int cap;
  if(team == 1)
    cap = 0;
  else
    cap = 1;
  
  for(i=0; i<strlen(buf); i++){
    if(cap && buf[i] == 'J')
      num++;
    else if(!cap && buf[i] == 'j')
      num++;
  }  

  return num;
}

int map_num_wall(char* buf)
{
  int num = 0;
  int i;

  for(i=0; i<strlen(buf); i++){
    if(buf[i] == '#')
      num++;

  }
  return num;
}

int map_num_floor(char* buf)
{
  int num = 0;
  int i;
  
  for(i=0; i<strlen(buf); i++){
    if(buf[i] == ' ' || buf[i] == 'j' || buf[i] == 'J'||\
       buf[i] == 'h' || buf[i] == 'H'){
      num++;
    }

  }
  return num;
}

extern int
load_map(Map* m)
{
  char noDim = 1;
  bzero(m,sizeof(m));
  if(!read_map(MAP_NAME)){
    fprintf(stderr,"could not read map at path %s\n",MAP_NAME);
    return 0;
  }
  char buf[MAX_LINE+1];

  bzero(buf, sizeof(buf));

  while(fgets(buf,COLUMN_MAX,fp) != NULL){
    if(noDim){
      m->dim = strlen(buf)-1;
      noDim = 0;
    }
    m->numhome1 += map_num_home(buf,1);
    m->numhome2 += map_num_home(buf,2);
    m->numjail1 += map_num_jail(buf,1);
    m->numjail2 += map_num_jail(buf,2);
    m->numwall +=  map_num_wall(buf);
    m->numfloor += map_num_floor(buf);
  }
  fclose(fp);
  //  m->dim = COLUMN_MAX;
  if(!read_map(MAP_NAME)){
    fprintf(stderr,"could not read map at path %s\n",MAP_NAME);
    return 0;
  }
  m->maze = (char**)load_maze();
  
  fclose(fp);
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
