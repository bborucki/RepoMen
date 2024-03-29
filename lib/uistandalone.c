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
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h> /* for exit() */
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include "types.h"
#include "uistandalone.h"

/* A lot of this code comes from http://www.libsdl.org/cgi/docwiki.cgi */

/* Forward declaration of some dummy player code */
static void dummyPlayer_init(UI *ui);
static void dummyPlayer_paint(UI *ui, SDL_Rect *t);

#define SPRITE_H 32
#define SPRITE_W 32

#define UI_FLOOR_BMP "./lib/images/floor.bmp"
#define UI_REDWALL_BMP "./lib/images/redwall.bmp"
#define UI_GREENWALL_BMP "./lib/images/greenwall.bmp"
#define UI_TEAMA_BMP "./lib/images/teama.bmp"
#define UI_TEAMB_BMP "./lib/images/teamb.bmp"
#define UI_LOGO_BMP "./lib/images/logo.bmp"
#define UI_REDFLAG_BMP "./lib/images/redflag.bmp"
#define UI_GREENFLAG_BMP "./lib/images/greenflag.bmp"
#define UI_JACKHAMMER_BMP "./lib/images/shovel.bmp"

typedef enum {UI_SDLEVENT_UPDATE, UI_SDLEVENT_QUIT} UI_SDL_Event;

struct UI_Player_Struct {
  SDL_Surface *img;
  uval base_clip_x;
  SDL_Rect clip;
};
typedef struct UI_Player_Struct UI_Player;

static inline SDL_Surface *
ui_player_img(UI *ui, int team){  
  return (team == 0) ? ui->sprites[TEAMA_S].img 
    : ui->sprites[TEAMB_S].img;
}

extern sval
ui_keypress(UI *ui, SDL_KeyboardEvent *e){
  SDLKey sym = e->keysym.sym;
  SDLMod mod = e->keysym.mod;
  
  // fprintf(stderr, "%c\n", sym);
  //  fprintf(stderr, "d\n",mod);
  if (e->type == SDL_KEYDOWN) {
    if (sym == SDLK_a && mod == KMOD_NONE) {
      fprintf(stderr, "%s: move left\n", __func__);
      return ui_dummy_left(ui);
    }
    if (sym == SDLK_d && mod == KMOD_NONE) {
      fprintf(stderr, "%s: move right\n", __func__);
      return ui_dummy_right(ui);
    }
    if (sym == SDLK_w && mod == KMOD_NONE)  {  
      fprintf(stderr, "%s: move up\n", __func__);
      return ui_dummy_up(ui);
    }
    if (sym == SDLK_s && mod == KMOD_NONE)  {
      fprintf(stderr, "%s: move down\n", __func__);
      return ui_dummy_down(ui);
    }
    if (sym == SDLK_e && mod == KMOD_NONE)  {  
      fprintf(stderr, "%s: dummy pickup red flag\n", __func__);
      return ui_dummy_pickup_red(ui);
    }
    if (sym == SDLK_f && mod == KMOD_NONE)  {   
      fprintf(stderr, "%s: dummy pickup green flag\n", __func__);
      return ui_dummy_pickup_green(ui);
    }
    if (sym == SDLK_j && mod == KMOD_NONE)  {   
      fprintf(stderr, "%s: dummy jail\n", __func__);
      return ui_dummy_jail(ui);
    }
    if (sym == SDLK_n && mod == KMOD_NONE)  {   
      fprintf(stderr, "%s: dummy normal state\n", __func__);
      return ui_dummy_normal(ui);
    }
    if (sym == SDLK_t && mod == KMOD_NONE)  {   
      fprintf(stderr, "%s: dummy toggle team\n", __func__);
      return ui_dummy_toggle_team(ui);
    }
    if (sym == SDLK_i && mod == KMOD_NONE)  {   
      fprintf(stderr, "%s: dummy inc player id \n", __func__);
      return ui_dummy_inc_id(ui);
    }
    if (sym == SDLK_q) return -1;
    if (sym == SDLK_z && mod == KMOD_NONE) return ui_zoom(ui, 1);
    if (sym == SDLK_z && mod & KMOD_SHIFT ) return ui_zoom(ui,-1);
    if (sym == SDLK_LEFT && mod & KMOD_SHIFT) return ui_pan(ui,-1,0);
    if (sym == SDLK_RIGHT && mod & KMOD_SHIFT) return ui_pan(ui,1,0);
    if (sym == SDLK_UP && mod & KMOD_SHIFT) return ui_pan(ui, 0,-1);
    if (sym == SDLK_DOWN && mod & KMOD_SHIFT) return ui_pan(ui, 0,1);
    else {
      fprintf(stderr, "%s: key pressed: %d\n", __func__, sym); 
    }
  } else {
    fprintf(stderr, "%s: key released: %d\n", __func__, sym);
  }
  return 1;
}

static inline sval 
pxSpriteOffSet(int team, int state){
  if (state == 1)
    return (team==0) ? SPRITE_W*1 : SPRITE_W*2;
  if (state == 2) 
    return (team==0) ? SPRITE_W*2 : SPRITE_W*1;
  if (state == 3) return SPRITE_W*3;
  return 0;
}

static sval
ui_uip_init(UI *ui, UI_Player **p, int id, int team){
  UI_Player *ui_p;
  
  ui_p = (UI_Player *)malloc(sizeof(UI_Player));
  if (!ui_p) return 0;

  ui_p->img = ui_player_img(ui, team);
  ui_p->clip.w = SPRITE_W; ui_p->clip.h = SPRITE_H; ui_p->clip.y = 0;
  ui_p->base_clip_x = id * SPRITE_W * 4;

  *p = ui_p;

  return 1;
}

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
static uint32_t 
ui_getpixel(SDL_Surface *surface, int x, int y){
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;
  
  switch (bpp) {
  case 1:
    return *p;
  case 2:
    return *(uint16_t *)p;
  case 3:
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      return p[0] << 16 | p[1] << 8 | p[2];
    else
      return p[0] | p[1] << 8 | p[2] << 16;
  case 4:
    return *(uint32_t *)p;
  default:
    return 0;       /* shouldn't happen, but avoids warnings */
  } // switch
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
static void 
ui_putpixel(SDL_Surface *surface, int x, int y, uint32_t pixel){
   int bpp = surface->format->BytesPerPixel;
   /* Here p is the address to the pixel we want to set */
   uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;

   switch (bpp) {
   case 1:
	*p = pixel;
	break;
   case 2:
     *(uint16_t *)p = pixel;
     break;     
   case 3:
     if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
       p[0] = (pixel >> 16) & 0xff;
       p[1] = (pixel >> 8) & 0xff;
       p[2] = pixel & 0xff;
     }
     else {
       p[0] = pixel & 0xff;
       p[1] = (pixel >> 8) & 0xff;
       p[2] = (pixel >> 16) & 0xff;
     }
     break;
 
   case 4:
     *(uint32_t *)p = pixel;
     break;
 
   default:
     break;           /* shouldn't happen, but avoids warnings */
   } // switch
}

static 
sval splash(UI *ui){
  SDL_Rect r;
  SDL_Surface *temp;


  temp = SDL_LoadBMP(UI_LOGO_BMP);
  
  if (temp != NULL) {
    ui->sprites[LOGO_S].img = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    r.h = ui->sprites[LOGO_S].img->h;
    r.w = ui->sprites[LOGO_S].img->w;
    r.x = ui->screen->w/2 - r.w/2;
    r.y = ui->screen->h/2 - r.h/2;
    //    printf("r.h=%d r.w=%d r.x=%d r.y=%d\n", r.h, r.w, r.x, r.y);
    SDL_BlitSurface(ui->sprites[LOGO_S].img, NULL, ui->screen, &r);
  } else {
    /* Map the color yellow to this display (R=0xff, G=0xFF, B=0x00)
       Note:  If the display is palettized, you must set the palette first.
    */
    r.h = 40;
    r.w = 80;
    r.x = ui->screen->w/2 - r.w/2;
    r.y = ui->screen->h/2 - r.h/2;
 
    /* Lock the screen for direct access to the pixels */
    if ( SDL_MUSTLOCK(ui->screen) ) {
      if ( SDL_LockSurface(ui->screen) < 0 ) {
	fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
	return -1;
      }
    }
    SDL_FillRect(ui->screen, &r, ui->yellow_c);

    if ( SDL_MUSTLOCK(ui->screen) ) {
      SDL_UnlockSurface(ui->screen);
    }
  }
  /* Update just the part of the display that we've changed */
  SDL_UpdateRect(ui->screen, r.x, r.y, r.w, r.h);

  SDL_Delay(1000);
  return 1;
}


static sval
load_sprites(UI *ui){
  SDL_Surface *temp;
  sval colorkey;

  /* setup sprite colorkey and turn on RLE */
  // FIXME:  Don't know why colorkey = purple_c; does not work here???
  colorkey = SDL_MapRGB(ui->screen->format, 255, 0, 255);
  
  temp = SDL_LoadBMP(UI_TEAMA_BMP);
  if (temp == NULL) { 
    fprintf(stderr, "ERROR: loading teama.bmp: %s", SDL_GetError()); 
    return -1;
  }
  ui->sprites[TEAMA_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(ui->sprites[TEAMA_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL, 
		  colorkey);

  temp = SDL_LoadBMP(UI_TEAMB_BMP);
  if (temp == NULL) { 
    fprintf(stderr, "ERROR: loading teamb.bmp: %s\n", SDL_GetError()); 
    return -1;
  }
  ui->sprites[TEAMB_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);

  SDL_SetColorKey(ui->sprites[TEAMB_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL, 
		  colorkey);
  temp = SDL_LoadBMP(UI_FLOOR_BMP);
  if (temp == NULL) {
    fprintf(stderr, "ERROR: loading floor.bmp %s\n", SDL_GetError()); 
    return -1;
  }
  ui->sprites[FLOOR_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(ui->sprites[FLOOR_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL, 
		  colorkey);

  temp = SDL_LoadBMP(UI_REDWALL_BMP);
  if (temp == NULL) { 
    fprintf(stderr, "ERROR: loading redwall.bmp: %s\n", SDL_GetError());
    return -1;
  }
  ui->sprites[REDWALL_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(ui->sprites[REDWALL_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL, 
		  colorkey);

  temp = SDL_LoadBMP(UI_GREENWALL_BMP);
  if (temp == NULL) {
    fprintf(stderr, "ERROR: loading greenwall.bmp: %s", SDL_GetError()); 
    return -1;
  }
  ui->sprites[GREENWALL_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(ui->sprites[GREENWALL_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL,
		  colorkey);

  temp = SDL_LoadBMP(UI_REDFLAG_BMP);
  if (temp == NULL) {
    fprintf(stderr, "ERROR: loading redflag.bmp: %s", SDL_GetError()); 
    return -1;
  }
  ui->sprites[REDFLAG_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(ui->sprites[REDFLAG_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL,
		  colorkey);

  temp = SDL_LoadBMP(UI_GREENFLAG_BMP);
  if (temp == NULL) {
    fprintf(stderr, "ERROR: loading redflag.bmp: %s", SDL_GetError()); 
    return -1;
  }
  ui->sprites[GREENFLAG_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(ui->sprites[GREENFLAG_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL,
		  colorkey);

  temp = SDL_LoadBMP(UI_JACKHAMMER_BMP);
  if (temp == NULL) {
    fprintf(stderr, "ERROR: loading %s: %s", UI_JACKHAMMER_BMP, SDL_GetError()); 
    return -1;
  }
  ui->sprites[JACKHAMMER_S].img = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(ui->sprites[JACKHAMMER_S].img, SDL_SRCCOLORKEY | SDL_RLEACCEL,
		  colorkey);
  
  return 1;
}

inline static void
draw_cell(UI *ui, SPRITE_INDEX si, SDL_Rect *t, SDL_Surface *s){
  SDL_Surface *ts=NULL;
  uint32_t tc;

  ts = ui->sprites[si].img;

  if ( ts && t->h == SPRITE_H && t->w == SPRITE_W) 
    SDL_BlitSurface(ts, NULL, s, t);
}

static sval
ui_paintmap(UI *ui){
  SDL_Rect t;
  int i, j, h, w, dim;
  t.y = 0; t.x = 0; t.h = ui->tile_h; t.w = ui->tile_w; 

  h = ui->screen->h;
  w = ui->screen->w;
  i = ((ui->uiplayer->x)/10)*10;
  j = ((ui->uiplayer->y)/10)*10;
  printf("x(i) = %d, y(j) = %d\n",ui->uiplayer->x,ui->uiplayer->y);

  for (t.y=0; t.y<h; t.y+=t.h, i++) {
    for (t.x=0, j = (ui->uiplayer->y/10)*10; t.x<w; t.x+=t.w, j++) {
      //      printf("ix = %d, jy = %d\n", i, j);
      if(cell_get_type(ui->uimap,i,j) == FLOOR){
	draw_cell(ui, FLOOR_S, &t, ui->screen);
      }else {
	draw_cell(ui, GREENWALL_S, &t, ui->screen);
      }
    }
  }

  dummyPlayer_paint(ui, &t);

  SDL_UpdateRect(ui->screen, 0, 0, ui->screen->w, ui->screen->h);
  return 1;
}

static sval
ui_init_sdl(UI *ui, int32_t h, int32_t w, int32_t d){
  fprintf(stderr, "UI_init: Initializing SDL.\n");

  /* Initialize defaults, Video and Audio subsystems */
  if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)==-1)) { 
    fprintf(stderr, "Could not initialize SDL: %s.\n", SDL_GetError());
    return -1;
  }

  atexit(SDL_Quit);

  fprintf(stderr, "ui_init: h=%d w=%d d=%d\n", h, w, d);

  ui->depth = d;
  ui->screen = SDL_SetVideoMode(w, h, ui->depth, SDL_SWSURFACE);
  if ( ui->screen == NULL ) {
    fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n", w, h, ui->depth, 
	    SDL_GetError());
    return -1;
  }
    
  fprintf(stderr, "UI_init: SDL initialized.\n");

  if (load_sprites(ui)<=0) return -1;

  ui->black_c      = SDL_MapRGB(ui->screen->format, 0x00, 0x00, 0x00);
  ui->white_c      = SDL_MapRGB(ui->screen->format, 0xff, 0xff, 0xff);
  ui->red_c        = SDL_MapRGB(ui->screen->format, 0xff, 0x00, 0x00);
  ui->green_c      = SDL_MapRGB(ui->screen->format, 0x00, 0xff, 0x00);
  ui->yellow_c     = SDL_MapRGB(ui->screen->format, 0xff, 0xff, 0x00);
  ui->purple_c     = SDL_MapRGB(ui->screen->format, 0xff, 0x00, 0xff);

  ui->isle_c         = ui->black_c;
  ui->wall_teama_c   = ui->red_c;
  ui->wall_teamb_c   = ui->green_c;
  ui->player_teama_c = ui->red_c;
  ui->player_teamb_c = ui->green_c;
  ui->flag_teama_c   = ui->white_c;
  ui->flag_teamb_c   = ui->white_c;
  ui->jackhammer_c   = ui->yellow_c;

  ui->x = 0;
  ui->y = 0;

  /* set keyboard repeat */
  SDL_EnableKeyRepeat(70, 70);  

  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
  SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
  SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

  splash(ui);
  return 1;
}

static void
ui_shutdown_sdl(void){
  fprintf(stderr, "UI_shutdown: Quitting SDL.\n");
  SDL_Quit();
}

static sval
ui_userevent(UI *ui, SDL_UserEvent *e) {
  if (e->code == UI_SDLEVENT_UPDATE) return 2;
  if (e->code == UI_SDLEVENT_QUIT) return -1;
  return 0;
}

static sval
ui_process(UI *ui){
  SDL_Event e;
  sval rc = 1;

  while(SDL_WaitEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT:
      return -1;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      //fprintf(stderr,"processing key\n");
      //rc = ui_keypress(ui, &(e.key));
      rc = 1;
      break;
    case SDL_ACTIVEEVENT:
      break;
    case SDL_USEREVENT:
      rc = ui_userevent(ui, &(e.user));
      break;
    default:
      fprintf(stderr, "%s: e.type=%d NOT Handled\n", __func__, e.type);
    }
    if (rc==2) { ui_paintmap(ui); }
    if (rc<0) break;
  }
  return rc;
}

extern sval
ui_zoom(UI *ui, sval fac){
  fprintf(stderr, "%s:\n", __func__);
  return 2;
}

extern sval
ui_pan(UI *ui, sval xdir, sval ydir){
  fprintf(stderr, "%s:\n", __func__);
  return 2;
}

extern sval
ui_move(UI *ui, sval xdir, sval ydir){
  fprintf(stderr, "%s:\n", __func__);
  fprintf(stderr, "xdir: %d, ydir: %d\n", xdir, ydir);
  return 1;
}

extern void
ui_update(UI *ui){
  SDL_Event event;
  
  event.type      = SDL_USEREVENT;
  event.user.code = UI_SDLEVENT_UPDATE;
  SDL_PushEvent(&event);
}


extern void
ui_quit(UI *ui){  
  SDL_Event event;
  fprintf(stderr, "ui_quit: stopping ui...\n");
  event.type      = SDL_USEREVENT;
  event.user.code = UI_SDLEVENT_QUIT;
  SDL_PushEvent(&event);
}

extern void
ui_main_loop(UI *ui, uval h, uval w){
  sval rc;
  
  assert(ui);

  ui_init_sdl(ui, h, w, 32);

  dummyPlayer_init(ui);

  ui_paintmap(ui);
   
  
  while (1) {
    if (ui_process(ui)<0) break;
  }

  ui_shutdown_sdl();
}


extern void
ui_init(UI **ui){
  *ui = (UI *)malloc(sizeof(UI));
  if (ui==NULL) return;

  bzero(*ui, sizeof(UI));
  
  (*ui)->tile_h = SPRITE_H;
  (*ui)->tile_w = SPRITE_W;

}


// Kludgy dummy player for testing purposes
struct DummyPlayerDesc {
  pthread_mutex_t lock;
  UI_Player *uip;
  int id;
  int x, y;
  int team;
  int state;
} dummyPlayer;

static void 
dummyPlayer_init(UI *ui) {
  pthread_mutex_init(&(dummyPlayer.lock), NULL);
  dummyPlayer.id = ui->uiplayer->id;
  dummyPlayer.x = ui->uiplayer->x; 
  dummyPlayer.y = ui->uiplayer->y; 
  dummyPlayer.team = ui->uiplayer->team;
  dummyPlayer.state = ui->uiplayer->state;
  ui_uip_init(ui, &dummyPlayer.uip, dummyPlayer.id, dummyPlayer.team); 
}

static void 
dummyPlayer_paint(UI *ui, SDL_Rect *t){
  int h = ui->screen->h;
  int w = ui->screen->w;

  int x = dummyPlayer.x;
  int y = dummyPlayer.y;

  printf("asd x = %d, y = %d asd\n", (x%10)*32, (y%10)*32);

  t->y = (y%10)*32; t->x = (x%10)*32;
  //t->y = dummyPlayer.y * t->h; t->x = dummyPlayer.x* t->w;
  printf("tx = %d, ty = %d\n", t->x, t->y);
  dummyPlayer.uip->clip.x = dummyPlayer.uip->base_clip_x +
      pxSpriteOffSet(dummyPlayer.team, dummyPlayer.state);
  SDL_BlitSurface(dummyPlayer.uip->img, &(dummyPlayer.uip->clip), ui->screen, t);
  pthread_mutex_unlock(&dummyPlayer.lock);
}

int
ui_dummy_left(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.x--;
    ui_move(ui, -1, 0);
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_right(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.x++;
    ui_move(ui, 1, 0);
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_down(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.y++;
    ui_move(ui, 0, -1);
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_up(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.y--;
    ui_move(ui, 0, 1);
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_normal(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.state = 0;
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_pickup_red(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.state = 1;
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_pickup_green(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.state = 2;
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}


int
ui_dummy_jail(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    dummyPlayer.state = 3;
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_toggle_team(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    if (dummyPlayer.uip) free(dummyPlayer.uip);
    dummyPlayer.team = (dummyPlayer.team) ? 0 : 1;
    ui_uip_init(ui, &dummyPlayer.uip, dummyPlayer.id, dummyPlayer.team);
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}

int
ui_dummy_inc_id(UI *ui){
  pthread_mutex_lock(&dummyPlayer.lock);
    if (dummyPlayer.uip) free(dummyPlayer.uip);
    dummyPlayer.id++;
    if (dummyPlayer.id>=100) dummyPlayer.id = 0;
    ui_uip_init(ui, &dummyPlayer.uip, dummyPlayer.id, dummyPlayer.team);
  pthread_mutex_unlock(&dummyPlayer.lock);
  return 2;
}
