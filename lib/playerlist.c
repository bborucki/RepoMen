#include <stdlib.h>
#include <string.h>
#include "player.h"

extern int
playerlist_create(PlayerList *pl){
  pl = (PlayerList *)malloc(sizeof(int)*MAX_PLAYERS);
  bzero(pl, sizeof(pl));
  return 1;
}
