/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

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
#define MAXWORMLEN 16
#define MAXWORMCOUNT 16
#define SPIDER_AFTER_WORMS 5
#define BUG_AFTER_WORMS 6

#define WAIT 1

typedef u8  Scalar;
typedef u8  Boolean;
typedef int Tile;
typedef u16 Joypad;
typedef unsigned int BigScalar;

// read tile from VRAM
#define LEVEL(x,y) *((Tile*)(vram + (2*(x)) + (80*(y) ) ))

// comparison tile pointers for VRAM
#define T_FREE (Tile)(Tiles + (t_black[2]         * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH1 (Tile)(Tiles + (t_mushroom1[2]     * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH2 (Tile)(Tiles + (t_mushroom2[2]     * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH3 (Tile)(Tiles + (t_mushroom3[2]     * TILE_WIDTH * TILE_HEIGHT ))
#define T_WORM (Tile)(Tiles + (t_wormbody[2]      * TILE_WIDTH * TILE_HEIGHT ))
#define T_WMHL (Tile)(Tiles + (t_wormheadleft[2]  * TILE_WIDTH * TILE_HEIGHT ))
#define T_WMHR (Tile)(Tiles + (t_wormheadright[2] * TILE_WIDTH * TILE_HEIGHT ))
#define T_PLYR (Tile)(Tiles + (t_player[2]        * TILE_WIDTH * TILE_HEIGHT ))
#define T_SHOT (Tile)(Tiles + (t_shot[2]          * TILE_WIDTH * TILE_HEIGHT ))
#define T_SPDR (Tile)(Tiles + (t_spider[2]        * TILE_WIDTH * TILE_HEIGHT ))
#define T_BUG  (Tile)(Tiles + (t_bug[2]           * TILE_WIDTH * TILE_HEIGHT ))

#define X_CENTERED(width) ((MAXX_SCREEN - (width)) / 2 - 1)
#define Y_CENTER (MAXY_SCREEN / 2 - 1)
	
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

Scalar spider_x, spider_y;

Scalar bug_x, bug_y;
Boolean bug_dirx, bug_diry;
Scalar bug_save;

Scalar wormkills_spider;
Scalar wormkills_bug;

BigScalar score;
#define SCORE_MUSHROOM 1
#define SCORE_WORMBODY 3
#define SCORE_WORMHEAD 5
#define SCORE_WORMHEAD_PERBODY 2
#define SCORE_SPIDER 7
#define SCORE_BUG 10

#define SCORE_X 0
#define SCORE_Y (MAXY_SCREEN - 1)
#define SCORE_WIDTH 10
char score_string[SCORE_WIDTH+1] = "0000000000";

const char* builddate = COMPILEDATE;

Tile tmp_level; // for repeated LEVEL() checks

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

void drawShot(){
  DrawMap(shot_x, shot_y, t_shot);
}

void drawPlayer(){
  DrawMap(player_x, player_y, t_player);
}

void drawSpider(){
  DrawMap(spider_x, spider_y, t_spider);
}

void drawBug(){
  DrawMap(bug_x, bug_y, t_bug);
}

void gameOver(){
  alive = 0;
}

// print a string using 7-segment display
void printString(Scalar x, Scalar y, const char *c){

  for (; *c; x++, c++) {
    switch (*c) {

    case '-':
      DrawMap(x, y, char_dash);
      break;

    case '.':
      DrawMap(x, y, char_underscore);
      break;

    case '/':
      DrawMap(x, y, char_slash);
      break;

    case 'A':
      DrawMap(x, y, char_A);
      break;

    case 'b':
    case 'B':
      DrawMap(x, y, char_B);
      break;

    case 'C':
      DrawMap(x, y, char_C);
      break;

    case 'd':
      DrawMap(x, y, char_D);
      break;

    case 'E':
      DrawMap(x, y, char_E);
      break;

    case 'F':
      DrawMap(x, y, char_F);
      break;

    case 'G':
      DrawMap(x, y, char_G);
      break;

    case 'i':
      DrawMap(x, y, char_I);
      break;

    case 'I':
      DrawMap(x, y, char_L);
      break;

    case 'l':
      DrawMap(x, y, char_L);
      break;

    case 'T':
      DrawMap(x, y, char_T);
      break;

    case 'u':
      DrawMap(x, y, char_U);
      break;

    case 'v':
      DrawMap(x, y, char_U);
      break;

    case '0':
      DrawMap(x, y, char_0);
      break;

    case '1':
      DrawMap(x, y, char_1);
      break;

    case '2':
      DrawMap(x, y, char_2);
      break;

    case '3':
      DrawMap(x, y, char_3);
      break;

    case '4':
      DrawMap(x, y, char_4);
      break;

    case '5':
      DrawMap(x, y, char_5);
      break;

    case '6':
      DrawMap(x, y, char_6);
      break;

    case '7':
      DrawMap(x, y, char_7);
      break;

    case '8':
      DrawMap(x, y, char_8);
      break;

    case '9':
      DrawMap(x, y, char_9);
      break;

    default:
      DrawMap(x, y, char_space);
      break;

    }
  }

}

// convert int->char
void scoreToString() {
  BigScalar tmp_score = score;
  char *c = score_string + SCORE_WIDTH;
  do {
    c--;
    *c = '0' + (tmp_score % 10);
    tmp_score = tmp_score / 10;
  } while (c != score_string);
}

// change, convert and display score
void addScore(Scalar add){
  score += add;
  scoreToString();
  printString( SCORE_X, SCORE_Y, score_string );
}

void getBugSave(){
  tmp_level = LEVEL(bug_x, bug_y);
  if (tmp_level == T_MSH1) {
    bug_save = 1;
  } else if (tmp_level == T_MSH2) {
    bug_save = 2;
  } else if (tmp_level == T_MSH3) {
    bug_save = 3;
  } else {
    bug_save = 0;
  }
}

void initBug(){
  if (rand()%2) {
    bug_x = 0;
    bug_dirx = true;
  } else {
    bug_x = MAXX - 1;
    bug_dirx = false;
  }
  bug_y = MAXY-2-(rand()%6);
  bug_diry = rand()%2;

  getBugSave();
  drawBug();
}

void initSpider(){
  spider_y = 0;

  do {
    spider_x = rand()%MAXY;
    tmp_level = LEVEL(spider_x, spider_y);
  } while (    tmp_level != T_FREE
	    && tmp_level != T_MSH1
	    && tmp_level != T_MSH2
	    && tmp_level != T_MSH3
	       ); // @FIXME lockup with super-long worms on first line

  TriggerFx(FX_SPIDERFALL, 0xdf, true);
  drawSpider();

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
	wormkills_spider++;
	wormkills_bug++;

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
      tmp_level = LEVEL(x,y);

      if (tmp_level == T_PLYR) {
	// got you!
	gameOver();
	
      } else if(tmp_level != T_FREE) {
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
      tmp_level = LEVEL(x,y);
      
      if (tmp_level == T_PLYR) {
	// got you!
	gameOver();
	
      } else if(tmp_level != T_FREE) {
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

  Joypad buttons = ReadJoypad(0);

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

    tmp_level = LEVEL(x,y);

    if (tmp_level == T_FREE) {

      drawEmpty(player_x, player_y);
      player_x = x;
      player_y = y;
      drawPlayer();

    } else if (tmp_level == T_WORM || tmp_level == T_WMHL || tmp_level == T_WMHL || tmp_level == T_SPDR || tmp_level == T_BUG ){

      gameOver();

    } else {
      
      // ran into something, don't move at all

    }
  }

}

void moveBug() {
  
  // replace old bug
  switch (bug_save) {

  case 1:
    drawMushroom1(bug_x, bug_y);
    break;

  case 2:
    drawMushroom2(bug_x, bug_y);
    break;

  case 3:
    drawMushroom3(bug_x, bug_y);
    break;

  default:
    drawEmpty(bug_x, bug_y);

  }

  // move bug
  if (bug_dirx) {
    bug_x++;
    if (bug_x == MAXX) {
      bug_x = bug_y = OFFSCREEN;
      return;
    }
  } else {
    if (bug_x == 0) {
      bug_x = bug_y = OFFSCREEN;
      return;
    }
    bug_x--;
  }
  if (bug_diry) {
    bug_y++;
    if (bug_y == MAXY - 1) {
      bug_diry = false;
    }
  } else {
    bug_y--;
    if (bug_y == MAXY - 8) {
      bug_diry = true;
    }
  }

  while (bug_y == 0); // DEBUG_FIND

  // draw bug
  getBugSave();
  drawBug();
  
  if (bug_x == player_x && bug_y == player_y) {
      // got you!
      gameOver();
    }

}

void moveSpider() {
  
  // replace old spider
  if (rand()%2) {
    drawMushroom1(spider_x, spider_y);
  } else {
    drawEmpty(spider_x, spider_y);
  }

  // move spider
  spider_y++;

  // draw / remove spider
  if (spider_y == MAXY) {
    drawEmpty(spider_x, spider_y - 1); // no mushrooms on base row
    spider_x = spider_y = OFFSCREEN;
  } else {
    drawSpider();

    if (spider_x == player_x && spider_y == player_y) {
      // got you!
      gameOver();
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
  tmp_level = LEVEL(shot_x, shot_y);

  if ( tmp_level == T_FREE ) {

    // draw bullet
    drawShot();

  } else if ( tmp_level == T_MSH1 ) {

    // damage mushroom, remove bullet
    TriggerFx(FX_MUSHROOM, 0xc0, true);
    drawMushroom2( shot_x, shot_y );
    shooting = 0;
    addScore(SCORE_MUSHROOM);

  } else if ( tmp_level == T_MSH2 ) {

    // damage mushroom, remove bullet
    TriggerFx(FX_MUSHROOM, 0xb0, true);
    drawMushroom3( shot_x, shot_y );
    shooting = 0;
    addScore(SCORE_MUSHROOM);

  } else if ( tmp_level == T_MSH3 ) {

    // damage mushroom, remove bullet
    TriggerFx(FX_MUSHROOM, 0xa0, true);
    drawEmpty( shot_x, shot_y );
    shooting = 0;
    addScore(SCORE_MUSHROOM);

  } else if ( tmp_level == T_WMHL || tmp_level == T_WMHR ) {

    // head shot, kill whole worm, remove bullet
    shootWormHead();
    shooting = 0;

  } else if ( tmp_level == T_WORM ) {

    // body shot, split worm, remove bullet
    shootWormBody();
    shooting = 0;

  } else if ( tmp_level == T_SPDR ) {

    // kill spider, create mushroom, remove bullet
    TriggerFx(FX_SPIDER, 0xe0, true);
    drawMushroom1(shot_x, shot_y);
    shooting = 0;
    addScore(SCORE_SPIDER);

    spider_x = spider_y = OFFSCREEN;

  } else if ( tmp_level == T_BUG ) {

    // kill bug, create mushroom, remove bullet
    TriggerFx(FX_SPIDER, 0xe0, true); // TODO: new sound!
    drawMushroom1(shot_x, shot_y);
    shooting = 0;
    addScore(SCORE_BUG);

    bug_x = bug_y = OFFSCREEN;

  } else if ( tmp_level == T_PLYR ) { // yeah, like he's fast enough

    // invincible, remove bullet
    shooting = 0;
  }

}

void drawBox(const Scalar xmin, const Scalar ymin, const Scalar xmax, const Scalar ymax, const Boolean clear){
  Scalar i;

  // corners are always drawn
  DrawMap( xmin, ymin, border_tl );
  DrawMap( xmax, ymin, border_tr );
  DrawMap( xmin, ymax, border_bl );
  DrawMap( xmax, ymax, border_br );

  // sides can be skipped on small boxes
  // OPTIMIZATION: don't check for this, we don't draw any small boxes
  for (i = xmin+1; i < xmax; i++) {
    DrawMap( i, ymin, border_top    );
    DrawMap( i, ymax, border_bottom );
  }
  for (i = ymin+1; i < ymax; i++) {
    DrawMap( xmin, i, border_left  );
    DrawMap( xmax, i, border_right );
  }

  if (clear) {
    Fill( xmin+1, ymin+1, xmax-xmin-1, ymax-ymin-1, 0 );
  }
}

void drawBorder(){
  drawBox( 0, 0, MAXX_SCREEN-1, MAXY_SCREEN-1, true );
}

void drawMapXCentered( const Scalar y, const VRAM_PTR_TYPE *tile, const Scalar width){
  DrawMap( X_CENTERED(width), y, tile );
}

void drawTitleScreen(){
  drawBorder();
  drawMapXCentered( 12, t_title, T_TITLE_WIDTH );
  drawMapXCentered( 20, t_selectcredits, T_SELECTCREDITS_WIDTH );
}

void drawCreditScreen(){
  drawBorder();
  DrawMap( 6, 4, t_title);
  DrawMap( 6, 6, t_copyright);
  DrawMap( 6, 7, t_gnugpl);
  DrawMap( 6, 8, t_url);

  // print compiledate
  printString( 3, MAXY - 3, "build" );
  printString( 3, MAXY - 2, VERSION );
  printString( 3, MAXY - 1, builddate );
}

void drawLevel(){
  clearScreen();
}

void drawGameOver(){
  // clearScreen();
  drawMapXCentered( Y_CENTER, t_gameover, T_GAMEOVER_WIDTH );
  printString( X_CENTERED(SCORE_WIDTH), Y_CENTER + 3, score_string );
  Fill( SCORE_X, SCORE_Y, SCORE_WIDTH, 1, 0 ); // remove score from bottom left
}

void joypadWaitForAnyRelease(){
  while (ReadJoypad(0) != 0) {};
}

void joypadWaitForAnyPress(){
  while (ReadJoypad(0) == 0) {};
}

void joypadWaitForAnyTap(){
  joypadWaitForAnyRelease();
  joypadWaitForAnyPress();
}

void creditScreen(){
  FadeOut(WAIT, 1);
  drawCreditScreen();
  FadeIn(WAIT, 1);

  // tap once to continue
  joypadWaitForAnyTap();
  
  FadeOut(WAIT, 1);
  drawTitleScreen();
  FadeIn(WAIT, 1);
}

void titleScreen(){

  drawTitleScreen();
  TriggerFx(FX_TITLESCREEN, 0xff, false);

  Joypad button = 0;

  while (button == 0) {

    int i;

    DrawMap( (MAXX - T_PRESSSTART_WIDTH) / 2 - 1, 16, t_pressstart);

    for (i = 0; button == 0 && i < 32000; i++) {
      button = ReadJoypad(0);
    }

    if (button == 0) {
      Fill( (MAXX - T_PRESSSTART_WIDTH) / 2 - 1, 16, T_PRESSSTART_WIDTH, T_PRESSSTART_HEIGHT, 0);

      for (i = 0; button == 0 && i < 32000; i++) {
	button = ReadJoypad(0);
      }
    }

    srand(i);

    if (button != BTN_START) {

      if (button == BTN_SELECT) {
	creditScreen();
      }

      button = 0; // stay on title screen

    }

  }

}

int main(){

  SetTileTable(Tiles);
  InitMusicPlayer(patches);

  titleScreen();

  while (1) {

    InitMusicPlayer(patches); // silence everything
    TriggerFx(FX_START, 0xff, false);

    // wait for button release
    joypadWaitForAnyRelease();

    // INIT GAME

    // init level
    drawLevel();

    // init worms
    wormcount = 0;
    wormkills_spider = 0;
    wormkills_bug = 0;

    // init spider
    spider_x = spider_y = OFFSCREEN;

    // init bug
    bug_x = bug_y = OFFSCREEN;

    // init mushrooms
    for (Scalar i = 0; i < 20; i++) {
      drawMushroom1( rand()%MAXX, rand()%(MAXY-1) );
    }
    
    // init player
    player_x = X_CENTERED(0); // center of line
    player_y = MAXY - 1; // last line
    drawPlayer();
    alive = 1;
    score = 0;
    addScore(0); // print initial score

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
      moveShot();

      if (spider_x != OFFSCREEN) {
	moveSpider();
      } else {
	if (wormkills_spider >= SPIDER_AFTER_WORMS) {
	  wormkills_spider = 0;
	  initSpider();
	}
      }
      
      WaitVsync(WAIT);
      movePlayer();
      moveShot();

      for (Scalar i = 1; i < MAXWORMCOUNT; i += 2) {
	moveWorm(i);
      }

      WaitVsync(WAIT);
      moveShot();

      if (bug_x != OFFSCREEN) {
	moveBug();
      } else {
	if (wormkills_bug >= BUG_AFTER_WORMS) {
	  wormkills_bug = 0;
	  initBug();
	}
      }
      
    }

    // GAME OVER

    InitMusicPlayer(patches); // silence everything
    TriggerFx(FX_GAMEOVER1, 0xff, false);
    TriggerFx(FX_GAMEOVER2, 0xbf, false);

    drawGameOver();

    // tap once to continue
    joypadWaitForAnyTap();

  }

}
