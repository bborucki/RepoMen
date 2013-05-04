#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "gamestate.h"
#include "cell.h"
#include "player.h"

#define MAXNUMCELLS 20
#define MAXNUMPLAYERS 20

extern int
gamestate_dump(Gamestate *g){
  int ncells, nplayers;
  int i = 0;
  
  printf("Dumping gamestate\n");

  ncells = g->numCells;
  nplayers = g->numPlayers;
  
  printf("There are %d players and %d cells\n", nplayers, ncells);

  while(nplayers){
    if(g->plist[i++] != NULL){
      player_dump(g->plist[i]);
      nplayers--;
    }
  }

  i = 0;
  while(ncells){
    if(g->clist[i++] != NULL){
      cell_dump(g->clist[i]);
      ncells--;
    }    
  }

  return 1;
}

extern Gamestate *
gamestate_create(){
  Gamestate *g;
  
  g = (Gamestate *)malloc(sizeof(Gamestate));
  bzero(g, sizeof(Gamestate));
  
  g->numCells = 0;
  g->numPlayers = 0;
  
  g->clist = (Cell **)malloc(sizeof(Cell *)*MAXNUMCELLS);
  g->plist = (Player **)malloc(sizeof(Player *)*MAXNUMPLAYERS);

  bzero(g->clist, sizeof(g->clist));
  bzero(g->plist, sizeof(g->plist));

  return g;
}

extern Player *
gamestate_get_player(Gamestate *g, int playerid){
  return g->plist[playerid];
}

extern int
gamestate_add_player(Gamestate *g, Player *p){
  printf("Adding a player!\n");

  if(g->numPlayers >= MAXNUMPLAYERS){
    return -1;
  }

  g->plist[p->id] = p;
  g->numPlayers++;

  printf("Added!\n");

  return 0;
}

//linear search for now
extern int
gamestate_remove_player(Gamestate *g, int playerid){
  if(g->numPlayers < 1)
    return -1;

  if(g->plist[playerid] == NULL)
    return -1;

  g->plist[playerid] = NULL;
  g->numPlayers--;

  return 1;
}

extern int
gamestate_add_cell(Gamestate *g, Cell *c){
  printf("Adding a cell!\n");

  if(g->numCells >= MAXNUMCELLS){
    return -1;
  }

  g->clist[c->idx] = c;
  g->numCells++;

  printf("Added!\n");

  return 0;
}

//another linear search for now
extern int
gamestate_remove_cell(Gamestate *g, int x, int y){
  int num = g->numCells;
  int i = 0;

  while(num){
    if(g->clist[i] != NULL){
      if(g->clist[i]->x == x && g->clist[i]->y == y){
	g->clist[i] = NULL;
	return 1;
      }
      num--;
    }
  }

  return -1;
}
