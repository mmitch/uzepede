#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>
#include <math.h>
#include <stdio.h>

#include "data/tiles.inc"

#define MAXX 40
#define MAXY 28
#define OFFSCREEN 255
#define MAXWORMLEN 32

#define WAIT 1

// read tile from VRAM
#define LEVEL(x,y) *((int*)(vram + (2*(x)) + (80*(y) ) ))

// comparison tile pointers for VRAM
#define T_FREE (int)(Tiles + (t_black[2]         * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH1 (int)(Tiles + (t_mushroom1[2]     * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH2 (int)(Tiles + (t_mushroom2[2]     * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH3 (int)(Tiles + (t_mushroom3[2]     * TILE_WIDTH * TILE_HEIGHT ))
#define T_WORM (int)(Tiles + (t_wormbody[2]      * TILE_WIDTH * TILE_HEIGHT ))
#define T_WMHL (int)(Tiles + (t_wormheadleft[2]  * TILE_WIDTH * TILE_HEIGHT ))
#define T_WMHR (int)(Tiles + (t_wormheadright[2] * TILE_WIDTH * TILE_HEIGHT ))
#define T_PLYR (int)(Tiles + (t_player[2]        * TILE_WIDTH * TILE_HEIGHT ))
#define T_SHOT (int)(Tiles + (t_shot[2]          * TILE_WIDTH * TILE_HEIGHT ))


typedef unsigned char Scalar;
typedef unsigned char Boolean;

typedef struct {
  Boolean direction;
  // worm has a ringbuffer of body elements
  // buffer "head" is the tail element
  Scalar tailidx;
  Scalar startidx;
  Scalar length;
} Worm;
  
Worm worms[2];
Scalar wormx[MAXWORMLEN];
Scalar wormy[MAXWORMLEN];
Scalar wormmax;

Scalar player_x, player_y, alive;
Scalar shot_x, shot_y, shooting;

void initLevel(){
  Fill(0, 0, MAXX, MAXY, 0);
}

void drawWormHead(Scalar x, Scalar y, Boolean direction){
  DrawMap(x, y, direction ? t_wormheadright : t_wormheadleft);
}

void drawWormBody(Scalar x, Scalar y){
  DrawMap(x, y, t_wormbody);
}

void drawEmpty(Scalar x, Scalar y){
  DrawMap(x, y, t_black);
}

void drawMushroom1(Scalar x, Scalar y){
  DrawMap(x, y, t_mushroom1);
}

void drawMushroom2(Scalar x, Scalar y){
  DrawMap(x, y, t_mushroom2);
}

void drawMushroom3(Scalar x, Scalar y){
  DrawMap(x, y, t_mushroom3);
}

void drawShot(Scalar x, Scalar y){
  DrawMap(x, y, t_shot);
}

void drawPlayer(Scalar x, Scalar y){
  DrawMap(x, y, t_player);
}

void gameOver(){
  alive = 0;
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
  newWorm->startidx = wormmax;
  drawWormHead(startx, starty, direction);

  for (Scalar i = wormmax; i < wormmax + length; i++) {
    wormx[i] = wormy[i] = OFFSCREEN;
  }
  wormx[wormmax] = startx;
  wormy[wormmax] = starty;

  wormmax += length;
}

void moveWorm(Scalar i){
  // move head, turn around if needed

  Scalar x, y;
  Worm *theWorm;

  theWorm = worms + i;

  // select old tail
  x = wormx[theWorm->tailidx];
  y = wormy[theWorm->tailidx];

  // delete old tail if onscreen
  if (x != OFFSCREEN) {
    drawEmpty(x, y);
  }

  // select old head
  if (theWorm->tailidx < theWorm->startidx + theWorm->length - 1){
    x = wormx[theWorm->tailidx+1];
    y = wormy[theWorm->tailidx+1];
  } else {
    x = wormx[theWorm->startidx];
    y = wormy[theWorm->startidx];
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

    if (moved) {

      if ((LEVEL(x,y) == T_PLYR)) {
	// got you!
	gameOver();
	
      } else if((LEVEL(x,y)) != T_FREE) {
	// can't go there
	moved = 0;
	x = oldx;

      } else {
	// ok, go here
      }

    }

    if (! moved) {
	y++;
	theWorm->direction = 1 - theWorm->direction;

      if ((LEVEL(x,y) == T_PLYR)) {
	// got you!
	gameOver();
	
      } else if((LEVEL(x,y)) != T_FREE) {
	// can't go there
	moved = 0;

      } else {
	// ok, go here
      }

    }
    
  }

  // handle overflow in Y direction
  if ( y >= MAXY ) {

    do {
      x = rand()%MAXX;
      y = rand()%3;
    } while ( LEVEL(x,y) != T_FREE );

  }

  wormx[theWorm->tailidx] = x;
  wormy[theWorm->tailidx] = y;

  // draw new head on current position
  drawWormHead(x, y, theWorm->direction);

  // advance tail through buffer
  if (theWorm->tailidx == theWorm->startidx) {
    theWorm->tailidx += theWorm->length;
  }
  theWorm->tailidx--;

}

void movePlayer(){

  Scalar x = player_x;
  Scalar y = player_y;

  int buttons = ReadJoypad(0);

  if ((buttons & BTN_LEFT)  && x > 0) {
    x--;
  }

  if ((buttons & BTN_RIGHT) && x < MAXX-1 ) {
    x++;
  }

  if ((buttons & BTN_UP)    && y > 0) {
    y--;
  }

  if ((buttons & BTN_DOWN)  && y < MAXY-1 ) {
    y++;
  }

  if ((buttons & BTN_A)     && ! shooting) {
    shooting = 1;
    shot_x = shot_y = OFFSCREEN;
  }

  if (player_x != x || player_y != y) {

    if ((LEVEL(x,y)) == T_FREE) {

      drawEmpty(player_x, player_y);
      drawPlayer(x, y);
      player_x = x;
      player_y = y;

    } else if ((LEVEL(x,y) == T_WORM || LEVEL(x,y) == T_WMHL || LEVEL(x,y) == T_WMHL)) {

      gameOver();

    } else {
      
      // ran into something, don't move at all

    }
  }

}

void moveShot(){
  
  if (! shooting) {
    return;
  }

  // remove old bullet / initialize
  if (shot_x == OFFSCREEN) {
    shot_x = player_x;
    shot_y = player_y;
  } else {
    drawEmpty( shot_x, shot_y );
  }

  // off screen?
  if (shot_y == 0) {
    shooting = 0;
  }

  // move shot
  shot_y--;
  
  // test for hit
  if ( LEVEL(shot_x, shot_y) == T_FREE ) {

    // draw bullet
    drawShot( shot_x, shot_y );

  } else if ( LEVEL(shot_x, shot_y) == T_MSH1 ) {

    // damage mushroom, remove bullet
    drawMushroom2( shot_x, shot_y );
    shooting = 0;

  } else if ( LEVEL(shot_x, shot_y) == T_MSH2 ) {

    // damage mushroom, remove bullet
    drawMushroom3( shot_x, shot_y );
    shooting = 0;

  } else if ( LEVEL(shot_x, shot_y) == T_MSH3 ) {

    // damage mushroom, remove bullet
    drawEmpty( shot_x, shot_y );
    shooting = 0;

  } else if ( LEVEL(shot_x, shot_y) == T_PLYR || LEVEL(shot_x, shot_y) == T_WORM ||
	      LEVEL(shot_x, shot_y) == T_WMHL || LEVEL(shot_x, shot_y) == T_WMHR ) {

    // invincible, remove bullet
    shooting = 0;
  }

}

int main(){

  SetTileTable(Tiles);

  // TITLE SCREEN

  Fill(0, 0, MAXX, MAXY, 0);
  DrawMap( (MAXX - T_TITLE_WIDTH) / 2 - 1, 13, t_title);

  int button = 0;

  while (button == 0) {

    int i;

    DrawMap( (MAXX - T_PRESSSTART_WIDTH) / 2 - 1, 18, t_pressstart);

    for (i = 0; button == 0 && i < 32000; i++) {
      button = ReadJoypad(0);
    }

    if (button == 0) {
      Fill( (MAXX - T_PRESSSTART_WIDTH) / 2 - 1, 18, T_PRESSSTART_WIDTH, T_PRESSSTART_HEIGHT, 0);

      for (i = 0; button == 0 && i < 32000; i++) {
	button = ReadJoypad(0);
      }
    }

    srand(i);

  }

  while (1) {

    // wait for button release
    while (ReadJoypad(0) != 0) {};

    // INIT GAME

    // init level
    initLevel();

    // init worms
    wormmax = 0;
    initWorm(0, 17, 6, 5, 1);
    initWorm(1, 23, 4, 9, 0);
    
    // init mushrooms
    for (Scalar i = 0; i < 20; i++) {
      drawMushroom1( rand()%MAXX, rand()%MAXY );
    }
    
    // init player
    player_x = MAXX / 2 - 1;
    player_y = MAXY - 1;
    drawPlayer(player_x, player_y);
    alive = 1;

    // init shot
    shooting = 0;
    shot_x = shot_y = OFFSCREEN;

    // GAME LOOP

    while(alive){
      
      WaitVsync(WAIT);
      
      movePlayer();
      moveWorm(1);
      moveShot();
      
      WaitVsync(WAIT);
      
      movePlayer();
      moveWorm(0);
      moveShot();
      
    }

    // GAME OVER

    Fill(0, 0, MAXX, MAXY, 0);
    DrawMap( (MAXX - T_GAMEOVER_WIDTH) / 2 - 1, MAXY / 2 - 1, t_gameover);

    // tap once to continue
    while (ReadJoypad(0) != 0) {};
    while (ReadJoypad(0) == 0) {};

  }

}
