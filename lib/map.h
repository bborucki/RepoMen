#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_client.h"

#define WALL 1
#define JAIL_1 2
#define JAIL_2 3
#define HOME_1 4
#define HOME_2 5
#define MAX_LINE  200
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



typedef struct Map{
  int numhome1;
  int numhome2;
  int numwall;
  int numfloor;
  int numjail1;
  int numjail2;
  int dim;
  char** maze;
} Map;

extern int load_map(Map* m);

extern Cell* make_cell(int ux, int uy);
