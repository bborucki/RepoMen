#ifndef __GAME_STATE__
#define __GAME_STATE__

#include "cell.h"
#include "player.h"
#include "types.h"

#define MAX_PLAYERS 198

struct Gamestate_ {
  char numCells;
  char numPlayers;
  Cell **clist;
  Player **plist;
};

extern Gamestate * gamestate_create();
extern int gamestate_add_cell(Gamestate *g, Cell *c);
extern int gamestate_remove_cell(Gamestate *g, int x, int y);
extern int gamestate_add_player(Gamestate *g, Player *p);
extern int gamestate_remove_player(Gamestate *g, int playerid);

#endif
