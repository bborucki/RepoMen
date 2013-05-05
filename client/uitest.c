/******************************************************************************
* Copyright (C) 2011 by Jonathan Appavoo, Boston University
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "../lib/types.h"
#include "../lib/tty.h"
#include "../lib/uistandalone.h"

UI *ui;

int
prompt(int menu) 
{
  static char MenuString[] = "\nclient> ";
  int ret;
  int c=0;

  if (menu) printf("%s", MenuString);
  fflush(stdout);
  c = getchar();
  return c;
}


int 
docmd(char cmd)
{
  int rc = 1;

  switch (cmd) {
  case 'q':
    printf("q ->quitting...\n");
    rc=-1;
    break;
  case 'w':
    printf("w ->do rpc: up\n");
    ui_dummy_up(ui);
    rc=2;
    break;
  case 's':
    printf("s ->do rpc: down\n");
    ui_dummy_down(ui);
    rc=2;
    break;
  case 'a':
    printf("a ->do rpc: left\n");
    ui_dummy_left(ui);
    rc=2;
    break;
  case 'd':
    printf("d ->do rpc: right\n");
    ui_dummy_right(ui);
    rc=2;
    break;
  case '\n':
    rc=1;
    break;
  default:
    printf("Unkown Command\n");
  }
  if (rc==2) ui_update(ui);
  return rc;
}

void *
shell(void *arg)
{
  char c;
  int rc;
  int menu=1;

  pthread_detach(pthread_self());

  while (1) {
    if ((c=prompt(menu))!=0) rc=docmd(c);
    if (rc<0) break;
    if (rc==1) menu=1; else menu=0;
  }

  fprintf(stderr, "terminating\n");
  fflush(stdout);
  ui_quit(ui);
  return NULL;
}

int
main(int argc, char **argv)
{
  pthread_t tid;

  tty_init(STDIN_FILENO);

  ui_init(&(ui));

  pthread_create(&tid, NULL, shell, NULL);

  // WITH OSX ITS IS EASIEST TO KEEP UI ON MAIN THREAD
  // SO JUMP THROW HOOPS :-(
  ui_main_loop(ui, 320, 320);

  return 0;
}
