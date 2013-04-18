#ifndef __MAP_H__
#define __MAP_H__

typedef struct Map{
  int numhome1;
  int numhome2;
  int numjail1;
  int numjail2;
  int numwall;
  int numfloor;
  int dim;
  char *maze;
} Map;

#define LINE_MAX 300
#define COLUMN_MAX 300
#define TEAM_1_MAX 100
#define TEAM_2_MAX 200

#define MAP_NAME "./lib/daGame.map"

extern Map * map_init(const char *path);

extern int map_dump(Map *m);

#endif
