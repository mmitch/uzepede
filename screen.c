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

static void clearScreen(){
  // clear whole mode 1 screen regardless of our internal screen size
  Fill(0, 0, 40, 28, 0);
}
static void drawWormHead(const Scalar x, const Scalar y, const Boolean direction_right){
  SetTile(x, y, direction_right ? TILE_WORMHEADRIGHT : TILE_WORMHEADLEFT);
}

static void drawWormBody(const Scalar x, const Scalar y){
  SetTile(x, y, TILE_WORMBODY);
}

static void drawEmpty(const Scalar x, const Scalar y){
  SetTile(x, y, TILE_FREE);
}

static void drawMushroom1(const Scalar x, const Scalar y){
  SetTile(x, y, TILE_MUSHROOM1);
}

static void drawMushroom2(const Scalar x, const Scalar y){
  SetTile(x, y, TILE_MUSHROOM2);
}

static void drawMushroom3(const Scalar x, const Scalar y){
  SetTile(x, y, TILE_MUSHROOM3);
}
