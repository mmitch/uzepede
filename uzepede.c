#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>
#include <math.h>
#include <stdio.h>

#include "data/tiles.inc"
#include "data/fonts.pic.inc"

#define MAXX 40
#define MAXY 28
#define OFFSCREEN 255
#define MAXWORMLEN 16

#define WAIT 1

#define T_FREE 0
#define T_MSH1 1
#define T_MSH2 2
#define T_MSH3 3
#define T_WORM 4

typedef unsigned char Scalar;
typedef unsigned char Boolean;

typedef struct {
  Boolean direction;
  // worm has a ringbuffer of body elements
  // buffer "head" is the tail element
  Scalar tailidx;
  Scalar length;
  Scalar x[MAXWORMLEN];
  Scalar y[MAXWORMLEN];
} Worm;
  
Worm worms[2];

unsigned char level[MAXX][MAXY];

void initLevel(){
  for (unsigned char x = 0; x < MAXX; x++) {
    for (unsigned char y = 0; y < MAXY; y++) {
      level[x][y] = T_FREE;
    }
  }
}

void drawWormHead(Scalar x, Scalar y, Boolean direction){
  DrawMap(x, y, direction ? t_wormheadright : t_wormheadleft);
  level[x][y] = T_WORM;
}

void drawWormBody(Scalar x, Scalar y){
  DrawMap(x, y, t_wormbody);
}

void drawEmpty(Scalar x, Scalar y){
  DrawMap(x, y, t_black);
  level[x][y] = T_FREE;
}

void drawMushroom(Scalar x, Scalar y){
  DrawMap(x, y, t_mushroom1);
  level[x][y] = T_MSH1;
}

void initWorm(Scalar i, Scalar startx, Scalar starty, Scalar length, Boolean direction){

  Worm *newWorm;

  newWorm = worms + i;

  newWorm->direction = direction;
  if (length > MAXWORMLEN) {
    length = MAXWORMLEN;
  }
  newWorm->length = length;
  newWorm->tailidx = length-1;
  if (direction) {
    for (Scalar i = 0; i < length; i++) {
      newWorm->x[i] = startx - i;
      newWorm->y[i] = starty;
      level[startx - i][starty] = T_WORM;
    }
  } else {
    for (Scalar i = 0; i < length; i++) {
      newWorm->x[i] = startx + i;
      newWorm->y[i] = starty;
      level[startx + i][starty] = T_WORM;
    }
  }
  drawWormHead(startx, starty, direction);
}

void moveWorm(Scalar i){
  // move head, turn around if needed

  Scalar x, y;
  Worm *theWorm;

  theWorm = worms + i;

  // select old tail
  x = theWorm->x[theWorm->tailidx];
  y = theWorm->y[theWorm->tailidx];

  // delete old tail if onscreen
  if (x != OFFSCREEN) {
    drawEmpty(x, y);
  }

  // select old head
  if (theWorm->tailidx < theWorm->length - 1){
    x = theWorm->x[theWorm->tailidx+1];
    y = theWorm->y[theWorm->tailidx+1];
  } else {
    x = theWorm->x[0];
    y = theWorm->y[0];
  }

  // draw body where the old head was
  if (x != OFFSCREEN) { // @TODO test should be superfluous!
    drawWormBody(x, y);
  }

  // compute new head position and store in old tail position
  Boolean moved = 0;
  Scalar oldx = x;
  while ( ! moved) {

    if (theWorm->direction) {
      if (x < MAXX - 1) {
	x++;
	moved = 1;
      }
    } else {
      if( x > 0 ) {
	x--;
	moved = 1;
      }
    }
    if (moved && level[x][y] != T_FREE) {
      moved = 0;
      x = oldx;
    }

    if (! moved) {
	y++;
	theWorm->direction = 1 - theWorm->direction;
	if (level[x][y] != T_FREE) {
	  moved = 0;
	}
    }

  }

  // handle overflow in Y direction -> @todo WARP ANYWHERE
  if ( y > 25 ) {

    do {
      x = rand()%MAXX;
      y = rand()%MAXY;
    } while ( level[x][y] != T_FREE );

  }

  theWorm->x[theWorm->tailidx] = x;
  theWorm->y[theWorm->tailidx] = y;

  // draw new head on current position
  drawWormHead(x, y, theWorm->direction);

  // advance tail through buffer
  if (theWorm->tailidx > 0) {
    theWorm->tailidx--;
  } else {
    theWorm->tailidx = theWorm->length - 1;
  }

}

int main(){

  SetFontTable(fonts);
  SetTileTable(Tiles);
  ClearVram();

  initWorm(0, 17, 7, 5, 1);
  initWorm(1, 23, 4, 9, 0);

  for (Scalar i = 0; i < 20; i++) {
    drawMushroom( rand()%MAXX, rand()%MAXY );
  }

  while(1){

    WaitVsync(WAIT);

    moveWorm(1);

    WaitVsync(WAIT);

    moveWorm(0);

  }
}
