/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
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

#include "data/patches.h"
#include "data/tiles.inc"

#include "types.h"
#include "util.h"


// leave this in as long as there is no memory pressure
#define SHOW_DEBUG_DATA_ON_ERROR

// playable area inside border
#define MINX 3
#define MINY 1
#define MAXX 37
#define MAXY 27

// whole screen with borders
#define MINX_SCREEN 2
#define MINY_SCREEN 0
#define MAXX_SCREEN 38
#define MAXY_SCREEN 28

#define OFFSCREEN 0xAA
#define MAXWORMLEN 16
#define MAXWORMCOUNT 16

// game parameters
#define SPIDER_AFTER_WORMS 5
#define BEE_AFTER_WORMS 6
#define INITIAL_MUSHROOMS 20

#define WAIT 1

// read tile from VRAM
#define LEVEL(x,y) *(vram + (x) + (40*(y) ))

// constants for the actual first tile content
// TODO: borders and characters are missing (except for c_slash only used once)
#define TILE_FREE          t_black[2]
#define TILE_MUSHROOM1     t_mushroom1[2]
#define TILE_MUSHROOM2     t_mushroom2[2]
#define TILE_MUSHROOM3     t_mushroom3[2]
#define TILE_WORMBODY      t_wormbody[2]
#define TILE_WORMHEADLEFT  t_wormheadleft[2]
#define TILE_WORMHEADRIGHT t_wormheadright[2]
#define TILE_PLAYER        t_player[2]
#define TILE_SHOT          t_shot[2]
#define TILE_SPIDER        t_spider[2]
#define TILE_BEE           t_bee[2]

#define X_CENTERED(width) ((MAXX_SCREEN - MINX_SCREEN - (width)) / 2 - 1 + MINX_SCREEN)
#define Y_CENTER ((MAXY_SCREEN + MINY_SCREEN) / 2 - 1 + MINY_SCREEN)

// last idx of a worm + 1
#define ENDIDX_PLUS_1(worm) ((worm)->startidx + (worm)->length)

#include "globals.h"

Scalar player_x, player_y, alive;

Scalar spider_x, spider_y;

Scalar bee_x, bee_y;
Boolean bee_dirx_right, bee_diry_down;
Boolean bee_over_mushroom;

#define SCORE_MUSHROOM 1
#define SCORE_WORMBODY 3
#define SCORE_WORMHEAD 5
#define SCORE_WORMHEAD_PERBODY 2
#define SCORE_SPIDER 7
#define SCORE_BEE 10

#define SCORE_X (MINX_SCREEN + 3)
#define SCORE_Y (MAXY_SCREEN - 1)
#define SCORE_WIDTH 10
char score_string[SCORE_WIDTH+1] = "0000000000";

#define DEBUG_REASON_SHOOT_WORM_BODY_MAXLEN_OVERFLOW   0x11
#define DEBUG_REASON_SHOOT_WORM_BODY_MAXWORM_OVERFLOW  0x22
#define DEBUG_REASON_MOVE_WORM_OLD_HEAD_OFFSCREEN      0x33

#ifdef SHOW_DEBUG_DATA_ON_ERROR

// we need some forward declarations
static void clearScreen();
static void printByte(const Scalar x, const Scalar y, Scalar value);
static void printString(Scalar x, const Scalar y, const char *c);

static void showDebugDataAndStopExecution(const Scalar val1, const Scalar val2, const Scalar val3, const Tile tile) {

  Scalar LINES_PER_COLUMN = 20;
  Scalar EXTRA_LINE_EVERY = 10;

  clearScreen();
  printString(3, 1, "DEBUG");

  SetTile  ( 4,  3, tile);
  SetTile  ( 5,  3, tile);
  printByte( 4,  4, val1);
  printByte( 4,  5, val2);
  printByte( 4,  6, val3);

  SetTile  ( 3,  8, TILE_SHOT);
  printByte( 4,  8, shot_x);
  SetTile  ( 6,  8, char_slash[2]);
  printByte( 7,  8, shot_y);

  SetTile  ( 3,  9, TILE_PLAYER);
  printByte( 4,  9, player_x);
  SetTile  ( 6,  9, char_slash[2]);
  printByte( 7,  9, player_y);

  SetTile  ( 3, 11, TILE_WORMHEADLEFT);
  printByte( 4, 11, MAXWORMCOUNT);
  SetTile  ( 3, 12, t_wormbody[2]);
  printByte( 4, 12, MAXWORMLEN);

  for (Scalar i = 0; i < MAXWORMCOUNT; i++) {
    Scalar y = i + 1 + (i / EXTRA_LINE_EVERY);
    SetTile  (10, y, TILE_WORMHEADLEFT);
    printByte(11, y, worms[i].startidx);
    SetTile  (13, y, TILE_WORMHEADLEFT);
    printByte(14, y, worms[i].tailidx);
    SetTile  (16, y, TILE_WORMBODY);
    printByte(17, y, worms[i].length);
  }

  for (Scalar i = 0; i < MAXWORMLEN; i++) {
    Scalar x = 20 + ( i / LINES_PER_COLUMN ) * 6;
    Scalar y = 1 + i % LINES_PER_COLUMN + (i % LINES_PER_COLUMN) / EXTRA_LINE_EVERY;
    printByte(x,   y, wormx[i]);
    SetTile  (x+2, y, char_slash[2]);
    printByte(x+3, y, wormy[i]);
  }

  while (1);
};

#else // SHOW_DEBUG_DATA_ON_ERROR

static void showDebugDataAndStopExecution(const Scalar val1, const Scalar val2, const Scalar val3, const Tile tile) {
  // debug disabled, do nothing
}

#endif // SHOW_DEBUG_DATA_ON_ERROR

// FIXME: including .c files is ugly!
// This is just a quick kludge to make the worm functions testable by themselves.
#include "screen.c"

static void drawShot(){
  SetTile(shot_x, shot_y, TILE_SHOT);
}

static void drawPlayer(){
  SetTile(player_x, player_y, TILE_PLAYER);
}

static void drawSpider(){
  SetTile(spider_x, spider_y, TILE_SPIDER);
}

static void drawBee(){
  SetTile(bee_x, bee_y, TILE_BEE);
}

static void gameOver(){
  alive = 0;
}

// print a string using 7-segment display
static void printString(Scalar x, const Scalar y, const char *c){

  for (; *c; x++, c++) {
    switch (*c) {

    case ' ':
      SetTile(x, y, char_space[2]);
      break;

    case '-':
      SetTile(x, y, char_dash[2]);
      break;

    case '.':
      SetTile(x, y, char_underscore[2]);
      break;

    case '/':
      SetTile(x, y, char_slash[2]);
      break;

    case 'a':
    case 'A':
      SetTile(x, y, char_A[2]);
      break;

    case 'b':
    case 'B':
      SetTile(x, y, char_B[2]);
      break;

    case 'c':
    case 'C':
      SetTile(x, y, char_C[2]);
      break;

    case 'd':
    case 'D':
      SetTile(x, y, char_D[2]);
      break;

    case 'e':
    case 'E':
      SetTile(x, y, char_E[2]);
      break;

    case 'f':
    case 'F':
      SetTile(x, y, char_F[2]);
      break;

    case 'g':
    case 'G':
      SetTile(x, y, char_G[2]);
      break;

    case 'i':
    case 'I':
      SetTile(x, y, char_I[2]);
      break;

    case 'l':
    case 'L':
      SetTile(x, y, char_L[2]);
      break;

    case 'T':
      SetTile(x, y, char_T[2]);
      break;

    case 'u':
    case 'U':
    case 'v':
    case 'V':
      SetTile(x, y, char_U[2]);
      break;

    case '0':
      SetTile(x, y, char_0[2]);
      break;

    case '1':
      SetTile(x, y, char_1[2]);
      break;

    case '2':
      SetTile(x, y, char_2[2]);
      break;

    case '3':
      SetTile(x, y, char_3[2]);
      break;

    case '4':
      SetTile(x, y, char_4[2]);
      break;

    case '5':
      SetTile(x, y, char_5[2]);
      break;

    case '6':
      SetTile(x, y, char_6[2]);
      break;

    case '7':
      SetTile(x, y, char_7[2]);
      break;

    case '8':
      SetTile(x, y, char_8[2]);
      break;

    case '9':
      SetTile(x, y, char_9[2]);
      break;

    default:
      SetTile(x, y, char_unknown[2]);
      break;

    }
  }

}

static char nibbleToChar(const Scalar nibble) {
  if (nibble < 0x0A) {
    return '0' + nibble;
  }
  return 'A' + nibble - 0x0A;
}

static void printByte(const Scalar x, const Scalar y, const Scalar value) {
  char *buf = "  ";
  buf[0] = nibbleToChar((value >> 4) & 0x0F);
  buf[1] = nibbleToChar( value       & 0x0F);
  printString(x, y, buf);
}

// convert int->char
static void scoreToString() {
  BigScalar tmp_score = score;
  char *c = score_string + SCORE_WIDTH;
  do {
    c--;
    *c = '0' + (tmp_score % 10);
    tmp_score = tmp_score / 10;
  } while (c != score_string);
}

// change, convert and display score
static void addScore(const Scalar add){
  score += add;
  scoreToString();
  printString( SCORE_X, SCORE_Y, score_string );
}

// To make triggerFx3() work, we have to expose TriggerCommon() from kernel/uzeboxSoundEngine.c
// This will of course break when the kernel changes, we're walking on thin ice here.
extern void TriggerCommon(Track* track,u8 patch,u8 volume,u8 note);

// This function is based on TriggerFx() from uzebox kernel/uzeboxSoundEngine.c
// It is drop-in compatible with the original.  In case of errors with future kernels,
// rename all calls to triggerFx3() to TriggerFx() and you are back to the original code.
//
// Uzepede only uses Fx, no music, thus our triggerFx3() has the following differences:
// - We want all 3 wave channels with voice stealing for Fx, not just 1+2 like in the kernel.
// - We also always want to retrigger, so remove the retrig check from the conditions.
// - We also don't use PCM (type == 2), so remove that branch.
static void triggerFx3(const unsigned char patch, const unsigned char volume, const bool _retrig){
  unsigned char channel;

  unsigned char type=(unsigned char)pgm_read_byte(&(patches[patch].type));

  //find the channel to play the fx
  //try to steal voice 2 then 1 then 0
  if(type==1){
    //noise channel fx
    channel=3;
  }else if( (tracks[0].flags&TRACK_FLAGS_PRIORITY)==0 || (tracks[0].fxPatchNo==patch && (tracks[0].flags&TRACK_FLAGS_PRIORITY)!=0)){ //fx already playing
    channel=0;
  }else if( (tracks[1].flags&TRACK_FLAGS_PRIORITY)==0 || (tracks[1].fxPatchNo==patch && (tracks[1].flags&TRACK_FLAGS_PRIORITY)!=0)){ //fx already playing
    channel=1;
  }else if( (tracks[2].flags&TRACK_FLAGS_PRIORITY)==0 || (tracks[2].fxPatchNo==patch && (tracks[2].flags&TRACK_FLAGS_PRIORITY)!=0)){ //fx already playing
    channel=2;
  }else{
    //all channels have fx playing, use the oldest one
    if(tracks[1].patchPlayingTime>tracks[2].patchPlayingTime){
      if(tracks[0].patchPlayingTime>tracks[1].patchPlayingTime){
	channel=0;
      }else{
	channel=1;
      }
    }else{
      if(tracks[0].patchPlayingTime>tracks[2].patchPlayingTime){
	channel=0;
      }else{
	      channel=2;
      }
    }
  }

  Track* track=&tracks[channel];
  track->flags=TRACK_FLAGS_PRIORITY; //priority=1;
  track->patchCommandStreamPos = NULL;
  TriggerCommon(track,patch,volume,80);
  track->flags|=TRACK_FLAGS_PLAYING;
}

static void getBeeSave(){
  bee_over_mushroom = (LEVEL(bee_x, bee_y) == TILE_MUSHROOM1)
                   || (LEVEL(bee_x, bee_y) == TILE_MUSHROOM2)
                   || (LEVEL(bee_x, bee_y) == TILE_MUSHROOM3);
}

static void initBee(){
  if (rand()%2) {
    bee_x = MINX;
    bee_dirx_right = true;
  } else {
    bee_x = MAXX - 1;
    bee_dirx_right = false;
  }
  bee_y = RAND_RANGE( MAXY-7, MAXY-2 );
  bee_diry_down = rand()%2;

  getBeeSave();
  drawBee();
  
  triggerFx3(FX_BEE_NEW, 0xdf, true);
}

static void initSpider(){
  spider_y = MINY;

  do {
    spider_x = RAND_RANGE( MINX, MAXX );
  } while (    LEVEL(spider_x, spider_y) != TILE_FREE
	    && LEVEL(spider_x, spider_y) != TILE_MUSHROOM1
	    && LEVEL(spider_x, spider_y) != TILE_MUSHROOM2
	    && LEVEL(spider_x, spider_y) != TILE_MUSHROOM3
	       ); // @FIXME lockup with super-long worms on first line

  triggerFx3(FX_SPIDERFALL, 0xdf, true);
  drawSpider();

}

// FIXME: including .c files is ugly!
// This is just a quick kludge to make the worm functions testable by themselves.
#include "worm.c"

static void movePlayer(){

  Scalar x = player_x;
  Scalar y = player_y;

  Joypad buttons = ReadJoypad(0);

  if ((buttons & BTN_LEFT)  && x > MINX) {
    x--;
  }

  if ((buttons & BTN_RIGHT) && x < MAXX-1 ) {
    x++;
  }

  if ((buttons & BTN_UP)    && y > MINY) {
    y--;
  }

  if ((buttons & BTN_DOWN)  && y < MAXY-1 ) {
    y++;
  }

  if ((buttons & BTN_A)     && ! shooting) {
    shooting = true;
    shot_x = shot_y = OFFSCREEN;
    triggerFx3(FX_SHOT, 0xd0, true);
  }

  if (player_x != x || player_y != y) {

    if (LEVEL(x,y) == TILE_FREE) {

      drawEmpty(player_x, player_y);
      player_x = x;
      player_y = y;
      drawPlayer();

    } else if (LEVEL(x,y) == TILE_WORMBODY || LEVEL(x,y) == TILE_WORMHEADLEFT || LEVEL(x,y) == TILE_WORMHEADRIGHT || LEVEL(x,y) == TILE_SPIDER || LEVEL(x,y) == TILE_BEE ){

      drawEmpty(player_x, player_y);
      player_x = x;
      player_y = y;
      drawPlayer();

      gameOver();

    } else {
      
      // ran into something, don't move at all

    }
  }

}

// kill bee, create mushroom, remove bullet
static void shootBee() {
  triggerFx3(FX_BEE_KILL, 0xe0, true);
  drawMushroom1(shot_x, shot_y);
  addScore(SCORE_BEE);
  bee_x = bee_y = OFFSCREEN;
  shooting = false;
}

static void moveBee() {
  
  // replace old bee
  if (bee_over_mushroom) {
    drawMushroom1(bee_x, bee_y);
  } else {
    drawEmpty(bee_x, bee_y);
  }

  // move bee
  if (bee_dirx_right) {
    bee_x++;
    if (bee_x == MAXX) {
      bee_x = bee_y = OFFSCREEN;
      return;
    }
  } else {
    if (bee_x == MINX) {
      bee_x = bee_y = OFFSCREEN;
      return;
    }
    bee_x--;
  }
  if (bee_diry_down) {
    bee_y++;
    if (bee_y == MAXY - 1) {
      bee_diry_down = false;
    }
  } else {
    bee_y--;
    if (bee_y == MAXY - 8) {
      bee_diry_down = true;
    }
  }

  // draw bee
  getBeeSave();
  drawBee();
  
  if (bee_x == player_x && bee_y == player_y) {
    // got you!
    gameOver();
  }

  // ran into shot -> selfkill
  if (IS_SHOT_AT(bee_x, bee_y)) {
    shootBee();
  }
}

// kill spider, create mushroom, remove bullet
static void shootSpider() {
  triggerFx3(FX_SPIDER, 0xe0, true);
  drawMushroom1(shot_x, shot_y);
  addScore(SCORE_SPIDER);
  spider_x = spider_y = OFFSCREEN;
  shooting = false;
}

static void moveSpider() {
  
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

    if (IS_SHOT_AT(spider_x, spider_y)) {
      shootSpider();
    } else {

      drawSpider();

      if (spider_x == player_x && spider_y == player_y) {
        // got you!
        gameOver();
      }
    }
  }

}

// damage mushroom, remove bullet
static void shootMushroom1() {
  triggerFx3(FX_MUSHROOM, 0xc0, true);
  drawMushroom2( shot_x, shot_y );
  addScore(SCORE_MUSHROOM);
  shooting = false;
}

// damage mushroom, remove bullet
static void shootMushroom2() {
  triggerFx3(FX_MUSHROOM, 0xb0, true);
  drawMushroom3( shot_x, shot_y );
  addScore(SCORE_MUSHROOM);
  shooting = false;
}

// remove mushroom, remove bullet
static void shootMushroom3() {
  triggerFx3(FX_MUSHROOM, 0xa0, true);
  drawEmpty( shot_x, shot_y );
  addScore(SCORE_MUSHROOM);
  shooting = false;
}

static void moveShot(){
  
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
  if (shot_y == MINY) {
    shooting = false;
    return;
  }

  // move shot
  shot_y--;
  
  // test for hit
  if ( LEVEL(shot_x, shot_y) == TILE_FREE ) {

    // draw bullet
    drawShot();

  } else if ( LEVEL(shot_x, shot_y) == TILE_MUSHROOM1 ) {

    shootMushroom1();

  } else if ( LEVEL(shot_x, shot_y) == TILE_MUSHROOM2 ) {

    shootMushroom2();

  } else if ( LEVEL(shot_x, shot_y) == TILE_MUSHROOM3 ) {

    shootMushroom3();

  } else if ( LEVEL(shot_x, shot_y) == TILE_WORMHEADLEFT || LEVEL(shot_x, shot_y) == TILE_WORMHEADRIGHT ) {

    shootWormHead();

  } else if ( LEVEL(shot_x, shot_y) == TILE_WORMBODY ) {

    shootWormBody();

  } else if ( LEVEL(shot_x, shot_y) == TILE_SPIDER ) {

    shootSpider();

  } else if ( LEVEL(shot_x, shot_y) == TILE_BEE ) {

    shootBee();

  } else if ( LEVEL(shot_x, shot_y) == TILE_PLAYER ) { // yeah, like he's fast enough

    // invincible, remove bullet
    shooting = false;
  }

}

static void drawBox(const Scalar xmin, const Scalar ymin, const Scalar xmax, const Scalar ymax, const Boolean clear){
  Scalar i;

  // corners are always drawn
  SetTile( xmin, ymin, border_tl[2] );
  SetTile( xmax, ymin, border_tr[2] );
  SetTile( xmin, ymax, border_bl[2] );
  SetTile( xmax, ymax, border_br[2] );

  // sides can be skipped on small boxes
  // OPTIMIZATION: don't check for this, we don't draw any small boxes
  for (i = xmin+1; i < xmax; i++) {
    SetTile( i, ymin, border_top[2]    );
    SetTile( i, ymax, border_bottom[2] );
  }
  for (i = ymin+1; i < ymax; i++) {
    SetTile( xmin, i, border_left[2]  );
    SetTile( xmax, i, border_right[2] );
  }

  if (clear) {
    Fill( xmin+1, ymin+1, xmax-xmin-1, ymax-ymin-1, 0 );
  }
}

static void drawBorder(){
  drawBox( MINX_SCREEN, MINY_SCREEN, MAXX_SCREEN-1, MAXY_SCREEN-1, true );
}

static void drawMapXCentered( const Scalar y, const VRAM_PTR_TYPE *tile, const Scalar width){
  DrawMap( X_CENTERED(width), y, tile );
}

static void drawTitleScreen(){
  drawBorder();
  drawMapXCentered( 12, t_title, T_TITLE_WIDTH );
  drawMapXCentered( 20, t_selectcredits, T_SELECTCREDITS_WIDTH );
}

static void drawCreditScreen(){
  drawBorder();

  const Scalar left_align = X_CENTERED( T_URL_WIDTH );
  DrawMap( left_align, MINY + 4, t_title);
  DrawMap( left_align, MINY + 6, t_copyright);
  DrawMap( left_align, MINY + 7, t_gnugpl);
  DrawMap( left_align, MINY + 8, t_url);

  // print compiledate
  printString( MINX, MAXY - 4, "build" );
  printString( MINX, MAXY - 3, COMPILEDATE );
  printString( MINX, MAXY - 2, VERSION );
  printString( MINX, MAXY - 1, GITCOMMIT );
}

static void drawLevel(){
  drawBorder();
  SetTile( SCORE_X - 1,           SCORE_Y, border_scorel[2] );
  SetTile( SCORE_X + SCORE_WIDTH, SCORE_Y, border_scorer[2] );
}

static void drawGameOver(){

  // redraw border to remove score from bottom left
  drawBox( MINX_SCREEN, MINY_SCREEN, MAXX_SCREEN-1, MAXY_SCREEN-1, false );

  // draw filled box in center
  const Scalar boxWidth = MAX( T_GAMEOVER_WIDTH, SCORE_WIDTH ) + 3;
  const Scalar boxLeft  = X_CENTERED( boxWidth );
  drawBox( boxLeft, Y_CENTER - 3, boxLeft + boxWidth, Y_CENTER + 3, true );

  // populate box
  drawMapXCentered( Y_CENTER - 1, t_gameover, T_GAMEOVER_WIDTH );
  printString( X_CENTERED(SCORE_WIDTH), Y_CENTER + 1, score_string );

}

static void joypadWaitForAnyRelease(){
  while (ReadJoypad(0) != 0) {};
}

static void joypadWaitForAnyPress(){
  while (ReadJoypad(0) == 0) {};
}

static void joypadWaitForAnyTap(){
  joypadWaitForAnyRelease();
  joypadWaitForAnyPress();
}

static void creditScreen(){
  FadeOut(WAIT, 1);
  drawCreditScreen();
  FadeIn(WAIT, 1);

  // tap once to continue
  joypadWaitForAnyTap();
  
  FadeOut(WAIT, 1);
  drawTitleScreen();
  FadeIn(WAIT, 1);
}

static void silenceAllSounds(){
  InitMusicPlayer(patches);
}

static void secretSoundTest(){
  drawBox(MINX+2, MINY+2, MINX+8, MINY+4, false);
  printString(MINX+3, MINY+3, "31337");
  silenceAllSounds();

  Joypad button = 0;
  while (button != BTN_START) {
    joypadWaitForAnyRelease();
    joypadWaitForAnyTap();
    button = ReadJoypad(0);

    switch(button) {
    case BTN_A:
      triggerFx3(FX_SHOT, 0xff, true);
      break;

    case BTN_B:
      triggerFx3(FX_MUSHROOM, 0xff, true);
      break;

    case BTN_X:
      triggerFx3(FX_WORMHEAD, 0xff, true);
      break;

    case BTN_Y:
      triggerFx3(FX_WORMBODY, 0xff, true);
      break;

    case BTN_UP:
      triggerFx3(FX_SPIDERFALL, 0xff, true);
      break;

    case BTN_DOWN:
      triggerFx3(FX_SPIDER, 0xff, true);
      break;

    case BTN_LEFT:
      triggerFx3(FX_BEE_NEW, 0xff, true);
      break;

    case BTN_RIGHT:
      triggerFx3(FX_BEE_KILL, 0xff, true);
      break;

    case BTN_SL:
      triggerFx3(FX_GAMEOVER1, 0xff, true);
      triggerFx3(FX_GAMEOVER2, 0xff, true);
      break;

    case BTN_SR:
      triggerFx3(FX_TITLESCREEN, 0xff, true);
      break;

    case BTN_SELECT:
      triggerFx3(FX_START, 0xff, true);
      break;
    };
  };
  joypadWaitForAnyRelease();

  Fill(MINX+2, MINY+2, MINX+8, MINY+4, 0);
}

static void titleScreen(){

  drawTitleScreen();
  triggerFx3(FX_TITLESCREEN, 0xff, true);

  Joypad button = 0;

  while (button == 0) {

    BigScalar i;

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

      if (button == (BTN_SL | BTN_SR)) {
        secretSoundTest();
      }

      button = 0; // stay on title screen

    }

  }

}

int main(){

  SetTileTable(Tiles);
  clearScreen();
  silenceAllSounds();

  titleScreen();

  while (1) {

    silenceAllSounds();
    triggerFx3(FX_START, 0xff, true);

    // wait for button release
    joypadWaitForAnyRelease();

    // INIT GAME

    // init level
    drawLevel();

    // init worms
    wormcount = 0;
    wormkills_spider = 0;
    wormkills_bee = 0;

    // init spider
    spider_x = spider_y = OFFSCREEN;

    // init bee
    bee_x = bee_y = OFFSCREEN;

    // init mushrooms
    for (Scalar i = 0; i < INITIAL_MUSHROOMS; i++) {
      drawMushroom1( RAND_RANGE( MINX, MAXX ), RAND_RANGE( MINY, MAXY-1 ) );
    }
    
    // init player
    player_x = X_CENTERED(0); // center of line
    player_y = MAXY - 1; // last line
    drawPlayer();
    alive = 1;
    score = 0;
    addScore(0); // print initial score

    // init shot
    shooting = false;
    shot_x = shot_y = OFFSCREEN;

    // GAME LOOP

    while(alive){

      if (wormcount == 0) {
	wormmax = 0;
	for (Scalar i = 0; i < MAXWORMCOUNT; i++) {
	  worms[i].length = 0;
	}
	for (Scalar i = 0; i < MAXWORMLEN; i++) {
	  wormx[i] = wormy[i] = OFFSCREEN;
	}
	initWorm(17, 6, 5, 1);
	initWorm(23, 4, 9, 0);
      }

      if (!alive) {
	      break;
      }

      WaitVsync(WAIT);
      movePlayer();
      moveShot();

      if (!alive) {
	      break;
      }

      for (Scalar i = 0; i < MAXWORMCOUNT; i += 2) {
	moveWorm(i);
      }
      
      if (!alive) {
	      break;
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
      
      if (!alive) {
	      break;
      }

      WaitVsync(WAIT);
      movePlayer();
      moveShot();

      if (!alive) {
	      break;
      }

      for (Scalar i = 1; i < MAXWORMCOUNT; i += 2) {
	moveWorm(i);
      }

      if (!alive) {
	      break;
      }

      WaitVsync(WAIT);
      moveShot();

      if (bee_x != OFFSCREEN) {
	moveBee();
      } else {
	if (wormkills_bee >= BEE_AFTER_WORMS) {
	  wormkills_bee = 0;
	  initBee();
	}
      }
      
      if (!alive) {
	      break;
      }

    }

    // GAME OVER

    silenceAllSounds();
    triggerFx3(FX_GAMEOVER1, 0xff, true);
    triggerFx3(FX_GAMEOVER2, 0xbf, true);

    drawGameOver();

    // tap once to continue
    joypadWaitForAnyTap();

  }

}
