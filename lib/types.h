#ifndef __DA_GAME_TYPES_H__
#define __DA_GAME_TYPES_H__
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

#include <stdint.h>

/* native word sized unsigned and signed values */
typedef unsigned long uval;
typedef long sval;
typedef enum {TEAM1, TEAM2} team_t;
typedef enum {SAFE, JAILED, FREE} state_t;
typedef enum {NONE, FLAG1, FLAG2, SHOVEL1, SHOVEL2} object_t;
typedef enum {FLOOR, JAIL1, JAIL2, HOME1, HOME2, WALL, IWALL} cell_t;
typedef enum {UP, DOWN, LEFT, RIGHT} dir_t;

typedef struct Gamestate_ Gamestate;
typedef struct Player_ Player;
typedef struct Cell_ Cell;
typedef struct ObjectMap_ ObjectMap;
//typedef PlayerList_ PlayerList;

#endif
