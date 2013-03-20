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


#define LINE_MAX  200
#define COLUMN_MAX 200
#define TEAM_1_MAX 100
#define TEAM_2_MAX 200
#define MAP_NAME "daGame.map"


int map_dump(const char* mappath);


int map_num_home(const char* mappath);

int map_num_jail(const char* mappath);

int map_num_wall(const char* mappath);

int map_num_floor(const char* mappath);

int map_dim(const char* mappath);

int map_cinfo(const char* mappath, int x, int y);
