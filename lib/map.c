#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

extern int
map_dump(Map *m){
  int i,j;
  int dim;
  
  dim = m->dim;

  for(i=0; i<dim; i++){
    for(j=0; j<dim; j++){
      printf("%c", m->maze[i*dim+j]);
    }
    //    printf("\n");
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

  dim = m->dim;

  for(i=0; i<dim; i++){
    for(j=0; j<dim; j++){
      switch(m->maze[i*dim+j]){
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

  printf("numall: %d\n", numFloor+numWall);
  printf("dim: %d\n", dim);
  printf("sizeof char %d\n", sizeof(char));
  printf("should malloc: %d\n", dim*dim*sizeof(char));
}

extern Map *
map_init(const char *path){
  Map *m = (Map *)malloc(sizeof(Map));
  bzero(m, sizeof(Map));
  char buf[COLUMN_MAX];
  int dim = 0;
  int i = 0;
  FILE *fp;
  int j;

  if((fp = fopen(path, "r")) == NULL)
    return NULL;

  while(fgets(buf, MAX_LINE, fp) != NULL){
    if(!dim){
      dim = strlen(buf);
      m->dim = dim;
      m->maze = (char *)malloc(sizeof(char)*dim*dim);
      bzero(m->maze, sizeof(char)*dim*dim);
      //      printf("%s\n", buf);
    }
    strncpy(&(m->maze[i*dim]), buf, dim);
    //    printf("%s", &(m->maze[i*dim]));
    i++;
  }
  
  //  map_dump(m);
  map_fill(m);

  printf("closing file\n");

  fclose(fp);
  
  printf("closed file\n");

  return m;
}
