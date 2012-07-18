#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>
#include <math.h>
#include <stdio.h>

#include "data/tiles.inc"
#include "data/patches.inc"

#define MAXX 40
#define MAXY 27
#define MAXX_SCREEN 40
#define MAXY_SCREEN 28
#define OFFSCREEN 255
#define MAXWORMLEN 32
#define MAXWORMCOUNT 32

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
  Scalar length;  // length == 0 -> worm is dead
  // @TODO: optimize: add endidx to cache (startidx + length - 1) ?
} Worm;
  
Worm worms[MAXWORMCOUNT];
Scalar wormx[MAXWORMLEN];
Scalar wormy[MAXWORMLEN];
Scalar wormmax;
Scalar wormcount;

Scalar player_x, player_y, alive;
Scalar shot_x, shot_y, shooting;

unsigned int score;
#define SCORE_MUSHROOM 1
#define SCORE_WORMBODY 5
#define SCORE_WORMHEAD 100
#define SCORE_WORMHEAD_PERBODY 10

void clearScreen(){
  Fill(0, 0, MAXX_SCREEN, MAXY_SCREEN, 0);
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

void print(Scalar x, Scalar y, unsigned int value){
  // @FIXME prints one position too the right

  for (Scalar nibble = 4; nibble > 0; nibble--) {
    Scalar displayNibble = value & 0x0F;
    value >>= 4;

    switch (displayNibble) {

    case 0:
      DrawMap(x + nibble, y, score_0);
      break;

    case 1:
      DrawMap(x + nibble, y, score_1);
      break;

    case 2:
      DrawMap(x + nibble, y, score_2);
      break;

    case 3:
      DrawMap(x + nibble, y, score_3);
      break;

    case 4:
      DrawMap(x + nibble, y, score_4);
      break;

    case 5:
      DrawMap(x + nibble, y, score_5);
      break;

    case 6:
      DrawMap(x + nibble, y, score_6);
      break;

    case 7:
      DrawMap(x + nibble, y, score_7);
      break;

    case 8:
      DrawMap(x + nibble, y, score_8);
      break;

    case 9:
      DrawMap(x + nibble, y, score_9);
      break;

    case 10:
      DrawMap(x + nibble, y, score_A);
      break;

    case 11:
      DrawMap(x + nibble, y, score_B);
      break;

    case 12:
      DrawMap(x + nibble, y, score_C);
      break;

    case 13:
      DrawMap(x + nibble, y, score_D);
      break;

    case 14:
      DrawMap(x + nibble, y, score_E);
      break;

    case 15:
      DrawMap(x + nibble, y, score_F);
      break;

    }
  }
}

void printScore(){
  print( 0, MAXY, score );
}

void addScore(Scalar add){
  score += add;
  printScore();
}

void initWorm(Scalar startx, Scalar starty, Scalar length, Boolean direction){

  Worm *newWorm = worms;

  if (wormcount >= MAXWORMCOUNT) {
    return;
  }

  while (newWorm->length > 0) {
    newWorm++;
  }

  newWorm->direction = direction;
  if (wormmax + length > MAXWORMLEN) {
    length = MAXWORMLEN - wormmax;
  }
  newWorm->length = length;
  newWorm->tailidx = wormmax + length - 1;
  newWorm->startidx = wormmax;
  drawWormHead(startx, starty, direction);

  for (Scalar i = wormmax; i < wormmax + length; i++) {
    wormx[i] = wormy[i] = OFFSCREEN;
  }
  wormx[wormmax] = startx;
  wormy[wormmax] = starty;

  wormmax += length;

  wormcount++;
}

void shootWormHead(){
  Worm *worm;
  Scalar i;

  // find worm that got shot
  for(worm = worms, i=0; i < MAXWORMCOUNT; worm++, i++) {
    if (worm->length) {

      // get head position
      Scalar idx = worm->tailidx + 1;
      if (idx == worm->startidx + worm->length) {
	idx = worm->startidx;
      }

      if (shot_x == wormx[idx] && shot_y == wormy[idx]){

	TriggerFx(FX_WORMHEAD, 0xef, true);

	// change worm to mushrooms
	for(i=0, idx=worm->startidx; i < worm->length; i++, idx++){
	  DrawMap( wormx[idx], wormy[idx], t_mushroom1 );
	  score += SCORE_WORMHEAD_PERBODY;
	}
	
	// kill worm
	worm->length = 0; 
	wormcount--;

	addScore(SCORE_WORMHEAD);

	break;
      }
    }
  }
    

}

void shootWormBody(){

  // find worm index that got hit
  Scalar idx;
  for (idx = 0; ! (wormx[idx] == shot_x && wormy[idx] == shot_y); idx++);

  // find worm that belongs to index
  Worm *worm;
  Scalar i;
  for (i = 0, worm = worms;
       ! (idx >= worm->startidx && idx < worm->startidx + worm->length) && (i < MAXWORMCOUNT);
       worm++, i++);

  if (i == MAXWORMCOUNT) {
    return;
  }

  TriggerFx(FX_WORMBODY, 0xdf, true);

  // calculate body part "number" that got shot (relative index to head)
  Scalar split = idx;
  if (split <= worm->tailidx) {
    split += worm->length;
  }
  split = split - worm->tailidx - 1;

  // rotate worm ringbuffer so head is at startidx
  while (worm->tailidx != worm->startidx + worm->length - 1) {
    Scalar tmpx, tmpy;
    Scalar i = worm->startidx + worm->length - 1;
    tmpx = wormx[i];
    tmpy = wormy[i];
    while (i > worm->startidx) {
      i--;
      wormx[i+1] = wormx[i];
      wormy[i+1] = wormy[i];
    }
    wormx[i] = tmpx;
    wormy[i] = tmpy;

    worm->tailidx++;
  }

  // create new worm of last part of there is something left
  if (split < worm->length - 1) {

    Worm *newWorm = worms;
    while (newWorm->length > 0) {
      newWorm++;
    }
    newWorm->startidx = worm->startidx + split + 1;
    newWorm->length = worm->length - split - 1;
    newWorm->direction = 1 - worm->direction;
    newWorm->tailidx = newWorm->startidx;
    wormcount++;
  }
    
  // shorten first part of worm
  worm->length = split;
  worm->tailidx = worm->startidx + worm->length - 1;
  
  // set mushroom on collision point, add score
  // shot body part is lost forever --> needs memory defragmentation :)
  drawMushroom1(shot_x, shot_y);
  addScore(SCORE_WORMBODY);
}

void moveWorm(Scalar i){
  // move head, turn around if needed

  Scalar x, y;
  Worm *theWorm;

  theWorm = worms + i;

  // don't move dead worms
  if (theWorm->length == 0) {
    return;
  }

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
  if (theWorm->length > 1) {
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
    } while ( LEVEL(x,y) != T_FREE ); // @FIXME will lock up when there are too many mushrooms in upper part

    // expand worm as much as possible
    Scalar newEnd = MAXWORMLEN;
    for (Scalar i = 0; i < MAXWORMCOUNT; i++) {
      if (worms + i != theWorm                     // not us
	  && worms[i].length > 0                   // alive
	  && worms[i].startidx > theWorm->startidx // behind us
	  && worms[i].startidx < newEnd            // but before new end
	  ) {
	newEnd = worms[i].startidx;
      }
    }
    for (Scalar i = theWorm->startidx + theWorm->length; i < newEnd; i++) {
      wormx[i] = OFFSCREEN;
      wormy[i] = OFFSCREEN;
    }
    theWorm->length = newEnd - theWorm->startidx;

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
    TriggerFx(FX_SHOT, 0xd0, true);
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
    return;
  }

  // move shot
  shot_y--;
  
  // test for hit
  if ( LEVEL(shot_x, shot_y) == T_FREE ) {

    // draw bullet
    drawShot( shot_x, shot_y );

  } else if ( LEVEL(shot_x, shot_y) == T_MSH1 ) {

    // damage mushroom, remove bullet
    TriggerFx(FX_MUSHROOM, 0xc0, true);
    drawMushroom2( shot_x, shot_y );
    shooting = 0;
    addScore(SCORE_MUSHROOM);

  } else if ( LEVEL(shot_x, shot_y) == T_MSH2 ) {

    // damage mushroom, remove bullet
    TriggerFx(FX_MUSHROOM, 0xb0, true);
    drawMushroom3( shot_x, shot_y );
    shooting = 0;
    addScore(SCORE_MUSHROOM);

  } else if ( LEVEL(shot_x, shot_y) == T_MSH3 ) {

    // damage mushroom, remove bullet
    TriggerFx(FX_MUSHROOM, 0xa0, true);
    drawEmpty( shot_x, shot_y );
    shooting = 0;
    addScore(SCORE_MUSHROOM);

  } else if ( LEVEL(shot_x, shot_y) == T_WMHL || LEVEL(shot_x, shot_y) == T_WMHR ) {

    // head shot, kill whole worm, remove bullet
    shootWormHead();
    shooting = 0;

  } else if ( LEVEL(shot_x, shot_y) == T_WORM ) {

    // body shot, split worm, remove bullet
    shootWormBody();
    shooting = 0;

  } else if ( LEVEL(shot_x, shot_y) == T_PLYR ) { // yeah, like he's fast enough

    // invincible, remove bullet
    shooting = 0;
  }

}

int main(){

  SetTileTable(Tiles);
  InitMusicPlayer(patches);

  // TITLE SCREEN

  clearScreen();
  DrawMap( (MAXX - T_TITLE_WIDTH) / 2 - 1, 13, t_title);
  TriggerFx(FX_TITLESCREEN, 0xff, false);

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

    InitMusicPlayer(patches); // silence everything
    TriggerFx(FX_START, 0xff, false);

    // wait for button release
    while (ReadJoypad(0) != 0) {};

    // INIT GAME

    // init level
    clearScreen();


    // init worms
    wormcount = 0;

    // init mushrooms
    for (Scalar i = 0; i < 20; i++) {
      drawMushroom1( rand()%MAXX, rand()%(MAXY-1) );
    }
    
    // init player
    player_x = MAXX / 2 - 1;
    player_y = MAXY - 1;
    drawPlayer(player_x, player_y);
    alive = 1;
    score = 0;
    printScore();

    // init shot
    shooting = 0;
    shot_x = shot_y = OFFSCREEN;

    // GAME LOOP

    while(alive){

      if (wormcount == 0) {
	wormmax = 0;
	for (Scalar i = 0; i < MAXWORMCOUNT; i++) {
	  worms[i].length = 0;
	}
	initWorm(17, 6, 5, 1);
	initWorm(23, 4, 9, 0);
      }

      WaitVsync(WAIT);
      movePlayer();
      moveShot();

      for (Scalar i = 0; i < MAXWORMCOUNT; i += 2) {
	moveWorm(i);
      }
      
      WaitVsync(WAIT);
      movePlayer();
      moveShot();

      for (Scalar i = 1; i < MAXWORMCOUNT; i += 2) {
	moveWorm(i);
      }
      
    }

    // GAME OVER

    InitMusicPlayer(patches); // silence everything
    TriggerFx(FX_GAMEOVER1, 0xff, false);
    TriggerFx(FX_GAMEOVER2, 0xbf, false);

    //    clearScreen();
    DrawMap( (MAXX - T_GAMEOVER_WIDTH) / 2 - 1, MAXY / 2 - 1, t_gameover);
    printScore();

    // tap once to continue
    while (ReadJoypad(0) != 0) {};
    while (ReadJoypad(0) == 0) {};

  }

}
