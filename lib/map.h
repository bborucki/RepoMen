#ifndef __MAP_H__
#define __MAP_H__

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

typedef enum {

  WALL,
  JAIL_1,
  JAIL_2,
  HOME_1,
  HOME_2

} Cell_Types;

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

extern int make_cell(Map* m, Cell * c, int ux, int uy);

#endif
