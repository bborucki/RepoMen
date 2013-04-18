#include <stdlib.h>
#include <string.h>
#include "player.h"

extern PlayerList *
playerlist_create(){
  PlayerList *pl = (PlayerList *)malloc(sizeof(Player)*MAX_PLAYERS);
  bzero(pl, sizeof(pl));
}
