#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "gamestate.h"
#include "cell.h"
#include "player.h"

#define MAXNUMCELLS 40000
#define MAXNUMPLAYERS 190


extern int
gamestate_team_wins(){
  if(flag1home1 == 1 && flag2home1 == 1 && numplayershome1 == numPlayers1)
    return TEAM1;
  if(flag1home2 == 1 && flag2home2 == 1 && numplayershome2 == numPlayers2)
    return TEAM2;
  else
    return -1;
}


extern int
gamestate_dump(Gamestate *g){
  int ncells; 
  int nplayers;
  int i = 0;
  
  printf("~~~~~~~~~~~~~~~~~~~~Dumping gamestate~~~~~~~~~~~~~~~~~~~~\n");

  ncells = g->numCells;
  nplayers = g->numPlayers;
  
  printf("There are %d players and %d cells\n", nplayers, ncells);

  while(nplayers && i < MAXNUMPLAYERS){
    if(g->plist[i] != NULL){
      player_dump(g->plist[i]);
      nplayers--;
    }
    i++;
  }

  i = 0;
  while(ncells && i < MAXNUMCELLS){
    if(g->clist[i] != NULL){
      cell_dump(g->clist[i]);
      ncells--;
    }    
    i++;
  }

  printf("~~~~~~~~~~~~~~~~~~~~Done dumping gamestate~~~~~~~~~~~~~~~~~~~~\n");

  return 1;
}

extern int
gamestate_move_player(Gamestate *g, int pid, int x, int y){
  Player *p;
  
  p = gamestate_get_player(g,pid);
  p->x = x;
  p->y = y;

  return 1;
}

extern Player *
gamestate_get_player(Gamestate *g, int playerid){
  return g->plist[playerid];
}

extern int
gamestate_add_player(Gamestate *g, Player *p){
  if(g->numPlayers >= MAXNUMPLAYERS){
    return -1;
  }

  g->plist[p->id] = p;
  g->numPlayers++;

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
  if(g->numCells >= MAXNUMCELLS){
    return -1;
  }

  g->clist[c->idx] = c;
  g->numCells++;

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

extern Gamestate *
gamestate_create(){
  Gamestate *g;
  int i;
  
  g = (Gamestate *)malloc(sizeof(Gamestate));
  bzero(g, sizeof(Gamestate));
  
  g->numCells = 0;
  g->numPlayers = 0;
  
  g->clist = (Cell **)malloc(sizeof(Cell *)*MAXNUMCELLS);
  g->plist = (Player **)malloc(sizeof(Player *)*MAXNUMPLAYERS);

  for(i = 0; i < MAXNUMCELLS; i++){
    g->clist[i] = NULL;    
  }
  for(i = 0; i < MAXNUMPLAYERS; i++){
    g->plist[i] = NULL;
  }

  return g;
}
