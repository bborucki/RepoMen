#include <stdio.h>
#include "map.h"

extern void
print_cell_type (Cell_Types ct){
  switch(ct){
  case WALL:
    printf("Wall\n");
    break;
  case JAIL_1:
    printf("Jail Team 1\n");
    break;
  case JAIL_2:
    printf("Jail Team 2\n");
    break;
  case HOME_1:
    printf("Home Team 1\n");
    break;    
  case HOME_2:
    printf("Home Team 2\n");
    break;    
  }
}
