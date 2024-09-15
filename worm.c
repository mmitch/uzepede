/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

#include "globals.h"
#include "util.h"

// last idx of a worm + 1
#define ENDIDX_PLUS_1(worm) ((worm)->startidx + (worm)->length)

static Boolean wormIsAlive(Worm *worm) {
  return worm->length > 0;
}

static Worm* findFirstFreeWorm() {
  Worm *deadWorm = worms;
  while (wormIsAlive(deadWorm)) {
    deadWorm++;
  }
  // FIXME: debug overflow here?  but currently all callers explcitely check beforehand
  return deadWorm;
}

static void initWorm(const Scalar startx, const Scalar starty, Scalar length, const Boolean direction_right){

  if (wormcount >= MAXWORMCOUNT) {
    return;
  }

  Worm *newWorm = findFirstFreeWorm();

  newWorm->direction_right = direction_right;
  if (wormmax + length > MAXWORMLEN) {
    length = MAXWORMLEN - wormmax;
  }
  newWorm->length = length;
  newWorm->tailidx = wormmax + length - 1;
  newWorm->startidx = wormmax;
  drawWormHead(startx, starty, direction_right);

  wormx[newWorm->startidx] = startx;
  wormy[newWorm->startidx] = starty;

  wormmax += length;

  wormcount++;
}

static void wormToMushrooms(const Scalar startidx, const Scalar endidx_exclusive) {
  for(Scalar idx = startidx; idx < endidx_exclusive; idx++) {
    drawMushroom1( wormx[idx], wormy[idx] );
    wormx[idx] = wormy[idx] = OFFSCREEN;
  }
}

static Scalar getWormHeadIdx(Worm *worm) {

  // ringbuffer: the head is one idx after the tail
  Scalar idx = worm->tailidx + 1;

  // wrap around if needed
  if (idx == ENDIDX_PLUS_1(worm)) {
    idx = worm->startidx;
  }

  return idx;
}

// head shot, kill whole worm, remove bullet
static void shootWormHead(){
  Worm *worm;

  // find worm that got shot
  for(worm = worms; worm < worms + MAXWORMCOUNT; worm++) {
    if (wormIsAlive(worm)) {

      // get head position
      Scalar idx = getWormHeadIdx(worm);

      if (IS_SHOT_AT(wormx[idx], wormy[idx])){

	triggerFx3(FX_WORMHEAD, 0xef, true);
	wormToMushrooms(worm->startidx, ENDIDX_PLUS_1(worm));

	// only one call to addScore(), because it adds *and displays* the score
	// TODO: refactor at least the name
	score += SCORE_WORMHEAD_PERBODY * (worm->length - 1);
	addScore(SCORE_WORMHEAD);
	
	// kill worm
	worm->length = 0; 
	wormcount--;
	wormkills_spider++;
	wormkills_bee++;

	shooting = false;

	break;
      }
    }
  }
}

// rotate worm ringbuffer so head is at startidx
static void rotateWormHeadToStartIdx(Worm *worm) {
  while (worm->tailidx != worm->startidx + worm->length - 1) {
    Scalar i = ENDIDX_PLUS_1(worm) - 1;
    Scalar tmpx = wormx[i];
    Scalar tmpy = wormy[i];
    while (i > worm->startidx) {
      i--;
      wormx[i+1] = wormx[i];
      wormy[i+1] = wormy[i];
    }
    wormx[i] = tmpx;
    wormy[i] = tmpy;

    worm->tailidx++;
  }
}

// body shot, split worm, remove bullet
static void shootWormBody(){

  // find worm index that got hit
  Scalar idx;
  for (idx = 0; (idx < MAXWORMLEN) && (! IS_SHOT_AT(wormx[idx], wormy[idx])); idx++);

  if (idx >= MAXWORMLEN) {
    // belt AND suspenders: this should never happen, but if it does, show why
    showDebugDataAndStopExecution(idx, 0, DEBUG_REASON_SHOOT_WORM_BODY_MAXLEN_OVERFLOW, TILE_WORMBODY);
    return;
  }

  // find worm that belongs to index
  Worm *worm;
  Scalar i;
  for (i = 0, worm = worms;
       ! (idx >= worm->startidx && idx < ENDIDX_PLUS_1(worm) && i < MAXWORMCOUNT);
       worm++, i++);

  if (i == MAXWORMCOUNT) {
    // belt AND suspenders: this should never happen, but if it does, show why
    showDebugDataAndStopExecution(idx, i, DEBUG_REASON_SHOOT_WORM_BODY_MAXWORM_OVERFLOW, TILE_WORMBODY);
    return;
  }

  // calculate body part "number" that got shot (relative index to head)
  Scalar split = idx;
  if (split <= worm->tailidx) {
    split += worm->length;
  }
  split = split - worm->tailidx - 1;

  // we need the head in front to be able to split properly
  rotateWormHeadToStartIdx(worm);

  // create new worm out of last part if there is something left
  if (split < worm->length - 1) {

    // this check is only relevant if MAXWORMCOUNT < MAXWORMLEN - 1
    // (ths - 1 is because one of the body parts of the old worm becomes a mushroom)
    // the compiler should remove this automatically depending on the compile flags
    if ((MAXWORMCOUNT < MAXWORMLEN - 1) && (wormcount >= MAXWORMCOUNT)) {
      // no free place for a new split worm, so make the last part into mushrooms instead
      wormToMushrooms(worm->startidx + split + 1, ENDIDX_PLUS_1(worm));

    } else {

      // create the new worm from the leftover part
      Worm *newWorm = findFirstFreeWorm();
      newWorm->startidx = worm->startidx + split + 1;
      newWorm->length = worm->length - split - 1;
      newWorm->direction_right = 1 - worm->direction_right;
      newWorm->tailidx = newWorm->startidx;
      wormcount++;
    }
  }
    
  // shorten first part of worm
  worm->length = split;
  worm->tailidx = ENDIDX_PLUS_1(worm) - 1;
  
  // set mushroom on collision point, add score
  // shot body part is lost forever --> this needs memory defragmentation,
  // which happens when all worms have been killed and new ones appear
  wormToMushrooms(worm->startidx + split, worm->startidx + split + 1);
  addScore(SCORE_WORMBODY);
  shooting = false;
}

static void moveWorm(const Scalar wormId){
  // move head, turn around if needed

  Scalar x, y;
  Worm *theWorm;

  theWorm = worms + wormId;

  // don't move dead worms
  if (! wormIsAlive(theWorm)) {
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
  const Scalar headIdx = getWormHeadIdx(theWorm);
  x = wormx[headIdx];
  y = wormy[headIdx];

  // draw body where the old head was
  if (theWorm->length > 1) {
    if (x == OFFSCREEN || y == OFFSCREEN) {
      // this should not be possible any more since we rotate a worm before enlarging it
      showDebugDataAndStopExecution(headIdx, wormId, DEBUG_REASON_MOVE_WORM_OLD_HEAD_OFFSCREEN, TILE_WORMHEADRIGHT);
    } else {
      drawWormBody(x, y);
    }
  }

  // compute new head position and store in old tail position
  Boolean moved = false;
  Scalar oldx = x;
  while ( ! moved) {

    if (theWorm->direction_right) {
      if (x < MAXX - 1) {
	x++;
	moved = 1;
      }
    } else {
      if( x > MINX ) {
	x--;
	moved = 1;
      }
    }

    if (moved) {

      if (LEVEL(x,y) == TILE_PLAYER) {
	// got you!
	gameOver();

      } else if(IS_SHOT_AT_TILED(x,y)) {
        // ok, go there, but be dead afterwards

      } else if(LEVEL(x,y) != TILE_FREE) {
	// can't go there
	moved = 0;
	x = oldx;

      } else {
	// ok, go here
      }

    }

    if (! moved) {
      y++;
      theWorm->direction_right = 1 - theWorm->direction_right;
      
      if (LEVEL(x,y) == TILE_PLAYER) {
	// got you!
	gameOver();
	
      } else if(IS_SHOT_AT_TILED(x,y)) {
        // ok, go there, but be dead afterwards

      } else if(LEVEL(x,y) != TILE_FREE) {
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
      x = RAND_RANGE( MINX+2, MAXX-2 );
      y = RAND_RANGE( MINY,   MINY+3 );
    } while ( LEVEL(x,y) != TILE_FREE ); // @FIXME will lock up when there are too many mushrooms in upper part

    // rotate before expanding the worm so that the head will not suddenly wrap into OFFSCREEN body parts
    rotateWormHeadToStartIdx(theWorm);

    // expand worm as much as possible
    Scalar newEnd = MAXWORMLEN;
    for (Scalar i = 0; i < MAXWORMCOUNT; i++) {
      if (worms + i != theWorm                     // not us
	  && wormIsAlive(worms + i)                // alive
	  && worms[i].startidx > theWorm->startidx // behind us
	  && worms[i].startidx < newEnd            // but before new end
	  ) {
	newEnd = worms[i].startidx;
      }
    }
    for (Scalar i = ENDIDX_PLUS_1(theWorm); i < newEnd; i++) {
      wormx[i] = OFFSCREEN;
      wormy[i] = OFFSCREEN;
    }
    theWorm->length = newEnd - theWorm->startidx;

  }

  wormx[theWorm->tailidx] = x;
  wormy[theWorm->tailidx] = y;

  // draw new head on current position
  drawWormHead(x, y, theWorm->direction_right);

  // advance tail through buffer
  if (theWorm->tailidx == theWorm->startidx) {
    theWorm->tailidx += theWorm->length;
  }
  theWorm->tailidx--;

  // the worm might have walked into a shot
  if (IS_SHOT_AT(x, y)) {
    shootWormHead();
  }
}
