#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "types.h"
#include "gamestate.h"
#include "objectmap.h"
#include "cell.h"
#include "map.h"

struct Player_ {
  int x;
  int y;
  char id;
  team_t team;
  object_t shovel;
  object_t flag;
  state_t state;
  Cell *pcell;
};

extern void player_dump(Player *p);
extern Player *player_create();
extern int player_move(dir_t dir, Player *p, ObjectMap *o, Gamestate *g);
extern int player_is_taggable(Player *tager, Player *tagee);
extern int player_tagHandler(Player *tagee, ObjectMap *o, Gamestate *g);
extern int player_find_empty_home(Player* p, team_t t, ObjectMap *o, int playerid);
extern int player_obj_pickup(Player* p, ObjectMap *o, Gamestate *g);
extern int player_find_next_id(Player **players);
#endif
