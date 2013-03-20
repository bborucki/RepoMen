#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_client.h"

FILE *fp;

int
read_map(const char* mappath)
{
  if((fp = fopen(mappath, "r")) == NULL)
    return 0;
  return 1;
}

int
map_dump(const char* mappath)
{
  char* buf = malloc(LINE_MAX);
  char* out = malloc(LINE_MAX);
  int i,j;
  if(!read_map(mappath)){
    fprintf(stderr, "could not read map with path %s\n",mappath);
    return 0;
  }
  while(fgets(buf,LINE_MAX, fp) != NULL){
    i = 0;
    /*
    while(i < TEAM_1_MAX){
      //  NYI designate team color;
      
    }
    while(i >= TEAM_1_MAX && i < TEAM_2_MAX){
      //  NYI designate team color;
    }
    */
    // this is temporary, but for now strcpy to out
    out = strcpy(out,buf);

    printf("%s",out);
  }
  
  return 1;
}

