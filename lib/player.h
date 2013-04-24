#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "types.h"
#include "objectmap.h"
#include "cell.h"
#include "playerlist.h"

struct Player_ {
  int id;
  team_t team;
  object_t shovel;
  object_t flag;
  state_t state;
  Cell *pcell;
};

extern int player_move(int x, int y, Player *p, ObjectMap *o, Map *m);
extern int player_is_taggable(Player *tager, Player *tagee);
extern int player_find_empty_home(Player* p, team_t t, ObjectMap *o, int playerid);
#endif
