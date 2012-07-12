#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>
#include <math.h>
#include <stdio.h>

#include "data/tiles.inc"

#define MAXX 30
#define MAXY 20
#define OFFSCREEN 255
#define MAXWORMLEN 12

#define WAIT 1

#define T_FREE 0
#define T_MSH1 1
#define T_MSH2 2
#define T_MSH3 3
#define T_WORM 4
#define T_PLYR 5
#define T_SHOT 6

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

unsigned char level[MAXX * MAXY];
// memory safeguard?!  something is wrong here!
#define LEVEL(x,y) level[ (x < MAXX ? x : MAXX - 1 ) + (y < MAXY ? y : MAXY - 1) * MAXX]
// #define LEVEL(x,y) level[x + y*MAXX]

Scalar player_x, player_y, alive;
Scalar shot_x, shot_y, shooting;

void initLevel(){
  for (unsigned char x = 0; x < MAXX; x++) {
    for (unsigned char y = 0; y < MAXY; y++) {
      LEVEL(x,y) = T_FREE;
    }
  }
}

void drawWormHead(Scalar x, Scalar y, Boolean direction){
  DrawMap(x, y, direction ? t_wormheadright : t_wormheadleft);
  LEVEL(x,y) = T_WORM;
}

void drawWormBody(Scalar x, Scalar y){
  DrawMap(x, y, t_wormbody);
}

void drawEmpty(Scalar x, Scalar y){
  DrawMap(x, y, t_black);
  LEVEL(x,y) = T_FREE;
}

void drawMushroom1(Scalar x, Scalar y){
  DrawMap(x, y, t_mushroom1);
  LEVEL(x,y) = T_MSH1;
}

void drawMushroom2(Scalar x, Scalar y){
  DrawMap(x, y, t_mushroom2);
  LEVEL(x,y) = T_MSH2;
}

void drawMushroom3(Scalar x, Scalar y){
  DrawMap(x, y, t_mushroom3);
  LEVEL(x,y) = T_MSH3;
}

void drawShot(Scalar x, Scalar y){
  DrawMap(x, y, t_shot);
  LEVEL(x,y) = T_SHOT;
}

void drawPlayer(Scalar x, Scalar y){
  DrawMap(x, y, t_player);
  LEVEL(x,y) = T_PLYR;
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
  newWorm->x[0] = startx;
  newWorm->y[0] = starty;

  drawWormHead(startx, starty, direction);

  for (Scalar i = 1; i < length; i++) {
    newWorm->x[i] = newWorm->y[i] = OFFSCREEN;
  }

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

    if (moved) {

      switch (LEVEL(x,y)) {
	
      case T_FREE:
	// ok, go here
	break;
	
      case T_PLYR:
	// got you!
	gameOver();
	break;
	
      default:
	// can't go there
	moved = 0;
	x = oldx;

      }
    }

    if (! moved) {
	y++;
	theWorm->direction = 1 - theWorm->direction;

	switch (LEVEL(x,y)) {

	case T_FREE:
	  // ok, go here
	  break;

	case T_PLYR:
	  // got you!
	  gameOver();
	  break;

	default:
	  // can't go there
	  moved = 0;
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

    switch (LEVEL(x,y)) {

    case T_FREE:
      drawEmpty(player_x, player_y);
      drawPlayer(x, y);
      player_x = x;
      player_y = y;
      break;
      
    case T_WORM:
      gameOver();
      
    default:
      // ran into something, don't move at all
      break;

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
  switch ( LEVEL(shot_x, shot_y) ) {

    // draw bullet
  case T_FREE:
    drawShot( shot_x, shot_y );
    break;

    // damage mushrooms, remove bullet
  case T_MSH1:
    drawMushroom2( shot_x, shot_y );
    shooting = 0;
    break;

  case T_MSH2:
    drawMushroom3( shot_x, shot_y );
    shooting = 0;
    break;

  case T_MSH3:
    drawEmpty( shot_x, shot_y );
    shooting = 0;
    break;

    // invincible, remove bullet
  case T_PLYR:
  case T_WORM:
    shooting = 0;
    break;
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

    Fill(0, 0, MAXX, MAXY, 0);

    // init level
    initLevel();

    // init worms
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
