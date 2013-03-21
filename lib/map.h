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

#define MAP_NAME "daGame.map"
#define OCCUPIED 1
#define UNOCCUPIED 0


int map_dump(const char* mappath);


int map_num_home(const char* mappath, int team);

int map_num_jail(const char* mappath, int team);

int map_num_wall(const char* mappath);

int map_num_floor(const char* mappath);

int map_dim(const char* mappath);

int map_cinfo(const char* mappath, int x, int y);
