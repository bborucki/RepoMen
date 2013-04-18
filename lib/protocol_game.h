#include "map.h"
#include "cell.h"
#include "net.h"
#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_server.h"
#include "protocol_session.h"


#define MAX_PLAYERS 198


static Map* Server_Map;
Cell *objects[LINE_MAX][COLUMN_MAX]; //2d array of map
Player *players[MAX_PLAYERS];
int playernext;
int numobjects;
