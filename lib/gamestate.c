#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "gamestate.h"
#include "cell.h"
#include "player.h"

#define MAXNUMCELLS 20
#define MAXNUMPLAYERS 20

extern Gamestate *
gamestate_create(){
  Gamestate *g;
  
  g = (Gamestate *)malloc(sizeof(Gamestate));
  bzero(g, sizeof(Gamestate));
  
  g->numCells = 0;
  g->numPlayers = 0;
  
  g->clist = (Cell **)malloc(sizeof(Cell)*MAXNUMCELLS);
  g->plist = (Player **)malloc(sizeof(Player)*MAXNUMPLAYERS);

  return g;
}

extern int
gamestate_add_player(Gamestate *g, Player *p){
  int rc = -1;
  int i = g->numPlayers;

  if(i < MAXNUMPLAYERS){
    if(g->plist[i] = NULL){
      g->plist[i] = p;
      g->numPlayers++;
      rc = 0; 
    }
  }

  return rc;
}

extern int
gamestate_remove_player(Gamestate *g, Player *p){
  

  return 0;
}

extern int
gamestate_add_cell(Gamestate *g, Cell *c){
  int rc = -1;
  int i = g->numCells;

  if(i < MAXNUMCELLS){
    if(g->clist[i] = NULL){
      g->clist[i] = c;
      g->numCells++;
      rc = 0; 
    }
  }

  return rc;
}

extern int
gamestate_remove_cell(Gamestate *g, Cell *c){


  return 0;
}
