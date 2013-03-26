#ifndef __MAZE_H__
#define __MAZE_H__

typedef struct Maze{
  int dim;
  char *data;
} Maze;

extern Maze * maze_init(const char *path);

#endif
