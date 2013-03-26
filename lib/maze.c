#include <stdlib.h>
#include <string.h>
#include "maze.h"
#include "map.h"

extern Maze *
maze_init(const char *path){
  Maze *m = (Maze *)malloc(sizeof(Maze));
  bzero(m, sizeof(Maze));
  char buf[COLUMN_MAX];
  int dim = 0;
  int i = 0;
  FILE *fp;
  int j;

  if((fp = fopen(path, "r")) == NULL)
    return NULL;

  while(fgets(buf, COLUMN_MAX, fp) != NULL){
    if(!dim){
      dim = strlen(buf)-1;
      m->dim = dim;
      m->data = malloc(sizeof(char)*dim*dim);
      bzero(m->data, sizeof(char)*dim*dim);
    }
    strncpy(&(m->data[i*dim]), buf, dim);
    i++;    
  }

  /*
    for(i = 0; i < dim; i++){
    for(j = 0; j < dim; j++){
    printf("%c", m->data[i*dim+j]);
    }
    printf("\n");
    }
  */
  fclose(fp);
  return m;
}
