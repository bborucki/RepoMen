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

typedef enum{
  WALL,
  JAIL_1,
  JAIL_2,
  HOME_1,
  HOME_2
} Cell_Types;

#define MAX_LINE 200
#define COLUMN_MAX 200
#define TEAM_1_MAX 100
#define TEAM_2_MAX 200

#define MAP_NAME "./lib/daGame.map"
#define OCCUPIED 1
#define UNOCCUPIED 0

typedef struct Cell{
  int type;
  int team;
  int occupied;
  int x;
  int y;
  int obj1;
  int obj2;
} Cell;

extern Map * map_init(const char *path);

extern int map_dump(Map *m);

extern int cell_create(Map *m, Cell *c, int x, int y);

#endif
