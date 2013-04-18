#ifndef __OBJECT_MAP__
#define __OBJECT_MAP__

#include "types.h"
#include "cell.h"
#include "map.h"
#include "player.h"
#include "playerlist.h"

#define MAX_PLAYERS 198

//define Player *Players[MAX_PLAYERS];

typedef struct ObjectMap{
  Cell **objects;
  int numPlayers;
  int dim;
} ObjectMap;

extern ObjectMap * objectmap_create(Map *m);
extern int objectmap_validate_move(int x, int y, Player *p, ObjectMap *o);
extern int objectmap_is_taggable(int x, int y, Player *p, ObjectMap *o, PlayerList *pl);

#endif
