#ifndef __GAME_STATE__
#define __GAME_STATE__

#include "cell.h"
#include "player.h"
#include "types.h"

#define MAX_PLAYERS 198

int gamefull;
int flag1found;
int flag2found;
int flag1home1;
int flag2home1;
int flag1home2;
int flag2home2;
int numplayershome1;
int numplayershome2;
int numPlayers1;
int numPlayers2;

struct Gamestate_ {
  int numCells;
  int numPlayers;
  Cell **clist;
  Player **plist;
};

extern Gamestate * gamestate_create();
extern int gamestate_move_player(Gamestate *g, int pid, int x, int y);
extern int gamestate_add_cell(Gamestate *g, Cell *c);
extern int gamestate_team_wins();
extern int gamestate_remove_cell(Gamestate *g, int x, int y);
extern int gamestate_add_player(Gamestate *g, Player *p);
extern int gamestate_remove_player(Gamestate *g, int playerid);
extern Player * gamestate_get_player(Gamestate *g, int playerid);

#endif
